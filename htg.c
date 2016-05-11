#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>

//--------------------------------------------------------------------------------------------------

// #define NDEBUG

#ifdef NDEBUG
#define DBG(M, ...)
#else
#define DBG(M, ...) fprintf(stderr, "DEBUG %s:%d: " M "", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

//--------------------------------------------------------------------------------------------------

// These command are used to retrive information from /proc/cpuinfo
// Number of physical cores = count(physical id) x cpu cores
// Number of logical cores = count(physycal id) x siblings
//
const char* cmd_cache = "cat /proc/cpuinfo  | grep 'cache size' | head -1 | awk '{ print $4 }'";

const char* cmd_num_pcores = "cat /proc/cpuinfo | grep 'core id' | sort | uniq | wc -l";
const char* cmd_num_lcores = "cat /proc/cpuinfo | grep 'core id' | sort | wc -l";

const char* cmd_num_cpu = "cat /proc/cpuinfo | grep 'physical id' | sort | uniq | wc -l";
const char* cmd_cores_per_cpu = "cat /proc/cpuinfo | grep 'cpu cores' | sort | uniq | awk '{ print $4 }'";
const char* cmd_threads_per_cpu = "cat /proc/cpuinfo | grep 'siblings' | sort | uniq | awk '{ print $3 }'";

// Helper function to execute commands
int execute(const char* cmd) {
  char output[1024];
  FILE* fp = popen(cmd, "r");
  fgets(output, sizeof(output) - 1, fp);
  return atoi(output);
}

//--------------------------------------------------------------------------------------------------

// We pass this struct to a thread
struct thread_arg_t {
  int thread_id_;
  // int num_threads_;

  // int num_lcores_;
  // int num_pcores_;
  // size_t llc_size_;

  int num_operations_;
  int completed_operations_;

  size_t buffer_length_;
};

//--------------------------------------------------------------------------------------------------

void* memcpy_thread(void* arg) {
  struct thread_arg_t* ta = (struct thread_arg_t*)arg;

  const size_t buffer_length = ta->buffer_length_;
  DBG("Thread-%d started doing %d memcpy operations (buffer length = %zu)\n",
      ta->thread_id_, ta->num_operations_, buffer_length);

  void* src = malloc(buffer_length / 2);
  memset(src, 0, buffer_length / 2);
  void* trg = malloc(buffer_length / 2);
  memset(trg, 1, buffer_length / 2);

  int i;
  for (i = 0; i < ta->num_operations_; i++) {
    memcpy(trg, src, buffer_length / 2);
    memcpy(src, trg, buffer_length / 2);  // HT issue only happens if this one is here too
  }

  DBG("Thread-%d: Completed operations = %d\n", ta->thread_id_, i);
  ta->completed_operations_ = i;
  pthread_exit(NULL);
}

//--------------------------------------------------------------------------------------------------

// Generic test function that starts given number of threads and measure
// the time they take to complete.
double runtest(size_t payload, size_t buffer_length, int num_threads, void* test()) {
  pthread_t threads[num_threads];
  struct thread_arg_t thread_args[num_threads];

  pthread_attr_t attr;
  /* Initialize and set thread detached attribute */
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  struct timeval t1, t2;
  gettimeofday(&t1, NULL);

  for (int t = 0; t < num_threads; t++) {
    thread_args[t].thread_id_ = t;
    thread_args[t].num_operations_ = payload / buffer_length / num_threads;
    thread_args[t].buffer_length_ = buffer_length;

    // pthread_create(&threads[t], &attr, calculate, &(thread_args[t]));
    pthread_create(&threads[t], NULL, test, &(thread_args[t]));
  }

  /* Free attribute and wait for the other threads */
  pthread_attr_destroy(&attr);

  int all_operations = 0;
  for (int t = 0; t < num_threads; t++) {
    void* status;
    int rc = pthread_join(threads[t], &status);
    if (rc) {
      DBG("ERROR; return code from pthread_join() is %d\n", rc);
      exit(-1);
    }

    all_operations += thread_args[t].completed_operations_;
    // printf("Thread-%d completed operations = %d\n", t, thread_args[t].completed_operations_);
    // printf("Main: completed join with thread %d having a status of %ld\n", t, (long)status);
  }

  DBG("All completed operations = %d\n", all_operations);
  DBG("All completed payload = %zu\n", all_operations * buffer_length);

  gettimeofday(&t2, NULL);
  double elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
  elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
  DBG("Elapsed time = %f ms\n", elapsedTime);

  return elapsedTime;
}

//--------------------------------------------------------------------------------------------------

void run_all_tests(size_t payload, int num_cpu, int num_threads, size_t cache_kb, void* test()) {
  size_t step = 32 * 1024;
  for (size_t buffer_length = step;
       buffer_length < 2 * num_cpu * cache_kb * 1024 / num_threads;
       buffer_length += step) {
    double elapsedtime = runtest(payload, buffer_length, num_threads, test);
    printf("buffer length = %zu elapsedtime = %f\n", buffer_length, elapsedtime);
    fflush(stdout);
    DBG("---------------------------------------------------------------------------\n");
  }
}

//--------------------------------------------------------------------------------------------------

int main(int argc, char *argv[]) {
  // int num_pcores = execute(cmd_num_pcores);
  // int num_lcores = execute(cmd_num_lcores);
  printf("HHH\n");
  int num_cpu = execute(cmd_num_cpu);

  int num_pcores = num_cpu * execute(cmd_cores_per_cpu);
  int num_lcores = num_cpu * execute(cmd_threads_per_cpu);

  size_t cache_kb = execute(cmd_cache) * num_cpu;

  DBG("LLC Cache = %zu KB\n", cache_kb);
  DBG("Number of physical cores = %d\n", num_pcores);
  DBG("Number of logical cores = %d\n", num_lcores);

  if (num_pcores == num_lcores) {
  } else if (num_lcores % num_pcores == 0) {
    if ( (num_lcores / num_pcores) > 1 )
      DBG("Hyperthreading is enabled: There are %d logical cores per a physical core.\n", num_lcores / num_pcores);
    else
      DBG("Hyperthreading is disabled: Number of logical cores equals the number of physical cores.\n");
  } else {
    DBG("Something is not OK. Number of logical cores is not multiplication of number of physical cores.\n");
    return 1;
  }

  DBG("---------------------------------------------------------------------------\n");


  // I want to move around size_t movearound = 10 * 1024 * 1024 * 1024 Bytes = 10 GB.
  size_t payload = 50l * 1024 * 1024 * 1024;
  run_all_tests(payload, num_cpu, num_pcores, cache_kb, memcpy_thread);

  /* Last thing that main() should do */
  pthread_exit(NULL);
  return 0;
}

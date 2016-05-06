#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>

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
  int num_threads_;

  int num_lcores_;
  int num_pcores_;
  size_t llc_size_;

  int num_operations_;
  int completed_operations_;
};

//--------------------------------------------------------------------------------------------------

void* memcpy_thread(void* arg) {
  struct thread_arg_t* ta = (struct thread_arg_t*)arg;

  const size_t buffer_length = ta->llc_size_ / ta->num_pcores_ / 2;
  printf("Thread-%d started doing %d memcpy operations"
         " (buffer length = cache_size / num_pcores / 2 = %zu)\n",
         ta->thread_id_, ta->num_operations_, buffer_length);

  void* src = malloc(buffer_length);
  memset(src, 0, buffer_length);
  void* trg = malloc(buffer_length);
  memset(trg, 1, buffer_length);

  int i;
  for (i = 0; i < ta->num_operations_; i++) {
    memcpy(trg, src, buffer_length);
    memcpy(src, trg, buffer_length);  // HT issue only happens if this one is here too
  }

  printf("Thread-%d: Completed operations = %d\n", ta->thread_id_, i);
  ta->completed_operations_ = i;
  pthread_exit(NULL);
}

//--------------------------------------------------------------------------------------------------

void* fpu_thread(void* arg) {
  struct thread_arg_t* ta = (struct thread_arg_t*)arg;

  printf("Thread-%d started doing %d FPU operations\n", ta->thread_id_, ta->num_operations_);

  int i;
  for (i = 0; i < ta->num_operations_; i++) {
    for (int j = 0; j < 10000; j++) {
      double a = sqrt(i);
      a = sqrt(a);
      a = pow(a, 2);
    }
  }

  printf("Thread-%d: Completed operations = %d\n", ta->thread_id_, i);
  ta->completed_operations_ = i;
  pthread_exit(NULL);
}

//--------------------------------------------------------------------------------------------------

void* integer_thread(void* arg) {
  struct thread_arg_t* ta = (struct thread_arg_t*)arg;

  printf("Thread-%d started doing %d integer operations\n", ta->thread_id_, ta->num_operations_);

  int i;
  for (i = 0; i < ta->num_operations_; i++) {
    for (int j = 0; j < 100000; j++) {
      int a = 2 * i + j;
      a = a * i + j;
      a++;
      a = a / 3;
    }
  }

  printf("Thread-%d: Completed operations = %d\n", ta->thread_id_, i);
  ta->completed_operations_ = i;
  pthread_exit(NULL);
}

//--------------------------------------------------------------------------------------------------

// Generic test function that starts given number of threads and measure
// the time they take to complete.
double runtest(int num_threads, int num_pcores, int num_lcores, size_t llc_size, void* test()) {
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
    thread_args[t].num_threads_ = num_threads;
    thread_args[t].num_lcores_ = num_lcores;
    thread_args[t].num_pcores_ = num_pcores;
    thread_args[t].llc_size_ = llc_size;
    thread_args[t].num_operations_ = num_pcores * 50000 / num_threads;

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
      printf("ERROR; return code from pthread_join() is %d\n", rc);
      exit(-1);
    }

    all_operations += thread_args[t].completed_operations_;
    // printf("Thread-%d completed operations = %d\n", t, thread_args[t].completed_operations_);
    // printf("Main: completed join with thread %d having a status of %ld\n", t, (long)status);
  }

  printf("All completed operations = %d\n", all_operations);

  gettimeofday(&t2, NULL);
  double elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
  elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
  printf("Elapsed time = %f ms\n", elapsedTime);

  return elapsedTime;
}

//--------------------------------------------------------------------------------------------------

void run_two_tests(int num_pcores, int num_lcores, size_t cache_kb, void* test()) {
  double lcores_elapsedtime = runtest(num_lcores, num_pcores, num_lcores, cache_kb * 1024, test);
  double pcores_elapsedtime = runtest(num_pcores, num_pcores, num_lcores, cache_kb * 1024, test);

  printf("lcores_elapsedtime = %f\n", lcores_elapsedtime);
  printf("pcores_elapsedtime = %f\n", pcores_elapsedtime);
  printf("lcores_elapsedtime / pcores_elapsedtime = %f\n", lcores_elapsedtime / pcores_elapsedtime);
  printf("---------------------------------------------------------------------------\n");
}

//--------------------------------------------------------------------------------------------------

int main(int argc, char *argv[]) {
  // int num_pcores = execute(cmd_num_pcores);
  // int num_lcores = execute(cmd_num_lcores);

  int num_cpu = execute(cmd_num_cpu);

  int num_pcores = num_cpu * execute(cmd_cores_per_cpu);
  int num_lcores = num_cpu * execute(cmd_threads_per_cpu);

  size_t cache_kb = execute(cmd_cache) * num_cpu;

  printf("LLC Cache = %zu KB\n", cache_kb);
  printf("Number of physical cores = %d\n", num_pcores);
  printf("Number of logical cores = %d\n", num_lcores);

  if (num_pcores == num_lcores) {
  } else if (num_lcores % num_pcores == 0) {
    if ( (num_lcores / num_pcores) > 1 )
      printf("Hyperthreading is enabled: There are %d logical cores per a physical core.\n", num_lcores / num_pcores);
    else
      printf("Hyperthreading is disabled: Number of logical cores equals the number of physical cores.\n");
  } else {
    printf("Something is not OK. Number of logical cores is not multiplication of number of physical cores.\n");
    return 1;
  }

  printf("---------------------------------------------------------------------------\n");

  run_two_tests(num_pcores, num_lcores, cache_kb, memcpy_thread);
  run_two_tests(num_pcores, num_lcores, cache_kb, fpu_thread);
  run_two_tests(num_pcores, num_lcores, cache_kb, integer_thread);

  /* Last thing that main() should do */
  pthread_exit(NULL);
  return 0;
}

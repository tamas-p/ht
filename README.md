# ht
Application to test what can make a program run slower if hyperthreading is enabled.

Example output:
LLC Cache = 8192 KB
Number of physical cores = 4
Number of logical cores = 8
Hyperthreading is enabled: There are 2 logical cores per a physical core.
---------------------------------------------------------------------------
Thread-0 started doing memcpy operations (buffer length = cache_size / num_pcores / 2 = 1048576)
Thread-3 started doing memcpy operations (buffer length = cache_size / num_pcores / 2 = 1048576)
Thread-2 started doing memcpy operations (buffer length = cache_size / num_pcores / 2 = 1048576)
Thread-4 started doing memcpy operations (buffer length = cache_size / num_pcores / 2 = 1048576)
Thread-1 started doing memcpy operations (buffer length = cache_size / num_pcores / 2 = 1048576)
Thread-5 started doing memcpy operations (buffer length = cache_size / num_pcores / 2 = 1048576)
Thread-6 started doing memcpy operations (buffer length = cache_size / num_pcores / 2 = 1048576)
Thread-7 started doing memcpy operations (buffer length = cache_size / num_pcores / 2 = 1048576)
Thread-0: Completed operations = 12500
Thread-3: Completed operations = 12500
Thread-2: Completed operations = 12500
Thread-5: Completed operations = 12500
Thread-7: Completed operations = 12500
Thread-4: Completed operations = 12500
Thread-1: Completed operations = 12500
Thread-6: Completed operations = 12500
All completed operations = 100000
Elapsed time = 8133.088000 ms
Thread-0 started doing memcpy operations (buffer length = cache_size / num_pcores / 2 = 1048576)
Thread-1 started doing memcpy operations (buffer length = cache_size / num_pcores / 2 = 1048576)
Thread-3 started doing memcpy operations (buffer length = cache_size / num_pcores / 2 = 1048576)
Thread-2 started doing memcpy operations (buffer length = cache_size / num_pcores / 2 = 1048576)
Thread-0: Completed operations = 25000
Thread-1: Completed operations = 25000
Thread-2: Completed operations = 25000
Thread-3: Completed operations = 25000
All completed operations = 100000
Elapsed time = 3007.567000 ms
lcores_elapsedtime = 8133.088000
pcores_elapsedtime = 3007.567000
lcores_elapsedtime / pcores_elapsedtime = 2.704208
---------------------------------------------------------------------------
Thread-1 started doing FPU operations
Thread-3 started doing FPU operations
Thread-0 started doing FPU operations
Thread-4 started doing FPU operations
Thread-2 started doing FPU operations
Thread-5 started doing FPU operations
Thread-7 started doing FPU operations
Thread-6 started doing FPU operations
Thread-5: Completed operations = 12500
Thread-0: Completed operations = 12500
Thread-1: Completed operations = 12500
Thread-4: Completed operations = 12500
Thread-3: Completed operations = 12500
Thread-2: Completed operations = 12500
Thread-6: Completed operations = 12500
Thread-7: Completed operations = 12500
All completed operations = 100000
Elapsed time = 2538.762000 ms
Thread-0 started doing FPU operations
Thread-2 started doing FPU operations
Thread-3 started doing FPU operations
Thread-1 started doing FPU operations
Thread-2: Completed operations = 25000
Thread-0: Completed operations = 25000
Thread-3: Completed operations = 25000
Thread-1: Completed operations = 25000
All completed operations = 100000
Elapsed time = 4079.953000 ms
lcores_elapsedtime = 2538.762000
pcores_elapsedtime = 4079.953000
lcores_elapsedtime / pcores_elapsedtime = 0.622253
---------------------------------------------------------------------------
Thread-0 started doing integer operations
Thread-1 started doing integer operations
Thread-5 started doing integer operations
Thread-4 started doing integer operations
Thread-7 started doing integer operations
Thread-6 started doing integer operations
Thread-3 started doing integer operations
Thread-2 started doing integer operations
Thread-4: Completed operations = 12500
Thread-1: Completed operations = 12500
Thread-0: Completed operations = 12500
Thread-6: Completed operations = 12500
Thread-3: Completed operations = 12500
Thread-7: Completed operations = 12500
Thread-5: Completed operations = 12500
Thread-2: Completed operations = 12500
All completed operations = 100000
Elapsed time = 8807.504000 ms
Thread-0 started doing integer operations
Thread-1 started doing integer operations
Thread-3 started doing integer operations
Thread-2 started doing integer operations
Thread-3: Completed operations = 25000
Thread-0: Completed operations = 25000
Thread-1: Completed operations = 25000
Thread-2: Completed operations = 25000
All completed operations = 100000
Elapsed time = 7686.427000 ms
lcores_elapsedtime = 8807.504000
pcores_elapsedtime = 7686.427000
lcores_elapsedtime / pcores_elapsedtime = 1.145852
---------------------------------------------------------------------------


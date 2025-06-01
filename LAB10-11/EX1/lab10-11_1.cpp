#include <pthread.h>   // For pthread_create, pthread_join
#include <stdio.h>     // For printf
#include <stdlib.h>    // For EXIT_SUCCESS, EXIT_FAILURE
#include <sys/time.h>  // For gettimeofday

// Shared resource
long long counter = 0;
pthread_mutex_t counter_mutex;  // Mutex to protect access to counter

// Structure to pass arguments to the thread function
typedef struct {
  int num_increments;
} thread_arg_t;

// Function that each thread will execute (synchronized)
void *increment_counter_synchronized(void *arg) {
  thread_arg_t *args = (thread_arg_t *)arg;
  for (int i = 0; i < args->num_increments; ++i) {
    pthread_mutex_lock(&counter_mutex);    // Acquire lock
    counter++;                             // Increment counter
    pthread_mutex_unlock(&counter_mutex);  // Release lock
  }
  pthread_exit(NULL);
}

int main() {
  const int K = 10;      // Number of threads
  const int N = 100000;  // Number of increments per thread

  pthread_t threads[K];
  thread_arg_t args = {.num_increments = N};

  struct timeval start_time, end_time;
  long seconds, microseconds;
  double elapsed_time_ms;

  // Initialize the mutex
  if (pthread_mutex_init(&counter_mutex, NULL) != 0) {
    fprintf(stderr, "Mutex initialization failed\n");
    return EXIT_FAILURE;
  }

  printf("Expected final value: %lld\n", (long long)K * N);

  // --- Start timing ---
  gettimeofday(&start_time, NULL);

  // Create and launch threads
  for (int i = 0; i < K; ++i) {
    if (pthread_create(&threads[i], NULL, increment_counter_synchronized, (void *)&args) != 0) {
      fprintf(stderr, "Error creating thread %d\n", i);
      // Clean up already created threads/mutex before exiting
      for (int j = 0; j < i; ++j) pthread_join(threads[j], NULL);
      pthread_mutex_destroy(&counter_mutex);
      return EXIT_FAILURE;
    }
  }

  // Wait for all threads to finish
  for (int i = 0; i < K; ++i) {
    if (pthread_join(threads[i], NULL) != 0) {
      fprintf(stderr, "Error joining thread %d\n", i);
      // Clean up mutex before exiting
      pthread_mutex_destroy(&counter_mutex);
      return EXIT_FAILURE;
    }
  }

  // --- End timing ---
  gettimeofday(&end_time, NULL);
  printf("Actual final value:   %lld\n", counter);

  if (counter != (long long)K * N) {
    printf("The value did not match. (Unexpected)\n");
  } else {
    printf("The value matched. (Expected)\n");
  }

  // Calculate elapsed time
  seconds = end_time.tv_sec - start_time.tv_sec;
  microseconds = end_time.tv_usec - start_time.tv_usec;
  elapsed_time_ms = (seconds * 1000.0) + (microseconds / 1000.0);

  printf("Execution time with mutex: %.3f ms\n", elapsed_time_ms);

  // Destroy the mutex
  if (pthread_mutex_destroy(&counter_mutex) != 0) {
    fprintf(stderr, "Mutex destruction failed\n");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

/*
Observations:
- Expected Value: The mathematically expected value is K * N (10 * 100,000 = 1,000,000).
- Actual Value Unsynchronized: Less than the expected value due to race conditions and lost updates.
- Actual Value Synchronized: Consistently equal to the expected value. This confirms correct synchronization.
- Execution Time Comparison:
    - Unsynchronized: Usually much faster.
    - Synchronized: Usually significantly slower.
- Reason for Time Difference: The synchronization (mutex locking and unlocking) introduces overhead. Threads have to
wait for the mutex to become available.
*/

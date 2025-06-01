#include <pthread.h>   // For pthread_create, pthread_join
#include <stdio.h>     // For printf
#include <stdlib.h>    // For EXIT_SUCCESS, EXIT_FAILURE
#include <sys/time.h>  // For gettimeofday

// Shared resource
long long counter = 0;

// Structure to pass arguments to the thread function
typedef struct {
  int num_increments;
} thread_arg_t;

// Function that each thread will execute (unsynchronized)
void *increment_counter_unsynchronized(void *arg) {
  thread_arg_t *args = (thread_arg_t *)arg;
  for (int i = 0; i < args->num_increments; ++i) {
    counter++;  // This operation is not atomic and leads to race conditions
  }
  pthread_exit(NULL);
}

int main() {
  const int K = 10;      // Number of threads
  const int N = 100000;  // Number of increments per thread

  pthread_t threads[K];                       // Array to hold thread IDs
  thread_arg_t args = {.num_increments = N};  // Arguments for threads

  struct timeval start_time, end_time;
  long seconds, microseconds;
  double elapsed_time_ms;

  printf("Expected final value: %lld\n", (long long)K * N);

  // --- Start timing ---
  gettimeofday(&start_time, NULL);

  // Create and launch threads
  for (int i = 0; i < K; ++i) {
    if (pthread_create(&threads[i], NULL, increment_counter_unsynchronized, (void *)&args) != 0) {
      fprintf(stderr, "Error creating thread %d\n", i);
      // Clean up already created threads before exiting
      for (int j = 0; j < i; ++j) pthread_join(threads[j], NULL);
      return EXIT_FAILURE;
    }
  }

  // Wait for all threads to finish
  for (int i = 0; i < K; ++i) {
    if (pthread_join(threads[i], NULL) != 0) {
      fprintf(stderr, "Error joining thread %d\n", i);
      return EXIT_FAILURE;
    }
  }

  // --- End timing ---
  gettimeofday(&end_time, NULL);
  printf("Actual final value:   %lld\n", counter);

  if (counter != (long long)K * N) {
    printf("The value did not match. (Expected)\n");
  } else {
    printf("The value matched. This is rare. (Unexpected))\n");
  }

  // Calculate elapsed time
  seconds = end_time.tv_sec - start_time.tv_sec;
  microseconds = end_time.tv_usec - start_time.tv_usec;
  elapsed_time_ms = (seconds * 1000.0) + (microseconds / 1000.0);

  printf("Execution time without mutex: %.3f ms\n", elapsed_time_ms);

  return EXIT_SUCCESS;
}

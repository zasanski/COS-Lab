#include <fcntl.h>     // For O_CREAT, O_RDWR
#include <stdio.h>     // For printf, fprintf, perror
#include <stdlib.h>    // For EXIT_SUCCESS, EXIT_FAILURE, exit
#include <sys/mman.h>  // For mmap, munmap
#include <sys/time.h>  // For gettimeofday
#include <sys/wait.h>  // For wait
#include <unistd.h>    // For fork, getpid, close, ftruncate

#include "shm_counter_common.h"

int main() {
  const int K = 10;
  const int N = 100000;

  int shm_fd;
  shared_data_t *shm_data;
  pid_t pids[K];

  struct timeval start_time, end_time;

  printf("--- Unsynchronized Counter (Processes + Shared Memory) ---\n");
  printf("Expected final value: %lld\n", (long long)K * N);

  shm_fd = shm_open(SHM_NAME_UNSYNCED, O_CREAT | O_RDWR, 0666);
  if (shm_fd == -1) {
    perror("shm_open failed");
    return EXIT_FAILURE;
  }

  if (ftruncate(shm_fd, sizeof(shared_data_t)) == -1) {
    perror("ftruncate failed");
    close(shm_fd);
    shm_unlink(SHM_NAME_UNSYNCED);
    return EXIT_FAILURE;
  }

  shm_data = (shared_data_t *)mmap(0, sizeof(shared_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (shm_data == MAP_FAILED) {
    perror("mmap failed");
    close(shm_fd);
    shm_unlink(SHM_NAME_UNSYNCED);
    return EXIT_FAILURE;
  }

  // Initialize the counter
  shm_data->counter = 0;

  // --- Start timing ---
  gettimeofday(&start_time, NULL);

  for (int i = 0; i < K; ++i) {
    pids[i] = fork();
    if (pids[i] == -1) {
      perror("fork failed");
      for (int j = 0; j < i; ++j) waitpid(pids[j], NULL, 0);
      munmap(shm_data, sizeof(shared_data_t));
      close(shm_fd);
      shm_unlink(SHM_NAME_UNSYNCED);
      return EXIT_FAILURE;
    } else if (pids[i] == 0) {
      // Child process
      for (int j = 0; j < N; ++j) {
        shm_data->counter++;  // Unsynchronized access
      }
      munmap(shm_data, sizeof(shared_data_t));
      close(shm_fd);
      exit(EXIT_SUCCESS);  // Child exits
    }
  }

  for (int i = 0; i < K; ++i) {
    waitpid(pids[i], NULL, 0);  // Parent waits for children
  }

  // --- End timing ---
  gettimeofday(&end_time, NULL);

  printf("Actual final value:   %lld\n", shm_data->counter);

  if (shm_data->counter != (long long)K * N) {
    printf("The value did not match. (Expected)\n");
  } else {
    printf("The value matched. This is rare. (Unexpected))\n");
  }

  long seconds = end_time.tv_sec - start_time.tv_sec;
  long microseconds = end_time.tv_usec - start_time.tv_usec;
  double elapsed_time_ms = (seconds * 1000.0) + (microseconds / 1000.0);
  printf("Execution time without semaphore: %.3f ms\n", elapsed_time_ms);

  munmap(shm_data, sizeof(shared_data_t));
  close(shm_fd);
  shm_unlink(SHM_NAME_UNSYNCED);

  return EXIT_SUCCESS;
}
/*
Observations:
Scenario 1: Unsynchronized Counter
- Result: Incorrect (less than expected) due to race condition and lost updates.
- Performance: Fastest (no synchronization overhead).
Scenario 2: Named Semaphore
- Result: Correct
- Performance: Slower than unsynchronized (overhead of locking/unlocking).
Scenario 3: Unnamed Semaphore
- Result: Correct.
- Performance: **Slower** than unsynchronized, but often slightly faster than named
semaphores due to avoiding system-wide name lookups and kernel object management.
*/

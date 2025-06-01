#include <errno.h>      // For errno
#include <fcntl.h>      // For O_CREAT, O_RDWR, O_EXCL
#include <semaphore.h>  // For sem_open, sem_close, sem_unlink, sem_wait, sem_post
#include <stdio.h>      // For printf, fprintf, perror
#include <stdlib.h>     // For EXIT_SUCCESS, EXIT_FAILURE, exit
#include <sys/mman.h>   // For mmap, munmap
#include <sys/time.h>   // For gettimeofday
#include <sys/wait.h>   // For wait
#include <unistd.h>     // For fork, getpid, close, ftruncate

#include "shm_counter_common.h"

int main() {
  const int K = 10;
  const int N = 100000;

  int shm_fd;
  shared_data_t *shm_data;
  sem_t *semaphore;
  pid_t pids[K];

  struct timeval start_time, end_time;
  long seconds, microseconds;
  double elapsed_time_ms;

  printf("--- Synchronized Counter (Processes + Shared Memory + Named Semaphore) ---\n");
  printf("Expected final value: %lld\n", (long long)K * N);

  sem_unlink(SEM_NAME_NAMED_SEM);  // Clean up any previous semaphore instance

  shm_fd = shm_open(SHM_NAME_NAMED_SEM, O_CREAT | O_RDWR, 0666);
  if (shm_fd == -1) {
    perror("shm_open failed");
    return EXIT_FAILURE;
  }
  if (ftruncate(shm_fd, sizeof(shared_data_t)) == -1) {
    perror("ftruncate failed");
    close(shm_fd);
    shm_unlink(SHM_NAME_NAMED_SEM);
    return EXIT_FAILURE;
  }
  shm_data = (shared_data_t *)mmap(NULL, sizeof(shared_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (shm_data == MAP_FAILED) {
    perror("mmap failed");
    close(shm_fd);
    shm_unlink(SHM_NAME_NAMED_SEM);
    return EXIT_FAILURE;
  }
  shm_data->counter = 0;  // Initialize counter

  // Open create semaphore
  semaphore = sem_open(SEM_NAME_NAMED_SEM, O_CREAT | O_EXCL, 0666, 1);  // Initial value 1
  if (semaphore == SEM_FAILED) {
    perror("sem_open failed");
    munmap(shm_data, sizeof(shared_data_t));
    close(shm_fd);
    shm_unlink(SHM_NAME_NAMED_SEM);
    return EXIT_FAILURE;
  }

  gettimeofday(&start_time, NULL);  // Start timing

  for (int i = 0; i < K; ++i) {
    pids[i] = fork();
    if (pids[i] == -1) {
      perror("fork failed");
      for (int j = 0; j < i; ++j) waitpid(pids[j], NULL, 0);
      sem_close(semaphore);
      sem_unlink(SEM_NAME_NAMED_SEM);
      munmap(shm_data, sizeof(shared_data_t));
      close(shm_fd);
      shm_unlink(SHM_NAME_NAMED_SEM);
      return EXIT_FAILURE;
    } else if (pids[i] == 0) {
      // Child process
      sem_t *child_semaphore = sem_open(SEM_NAME_NAMED_SEM, 0);  // Open existing
      if (child_semaphore == SEM_FAILED) {
        perror("Child: sem_open failed");
        munmap(shm_data, sizeof(shared_data_t));
        close(shm_fd);
        exit(EXIT_FAILURE);
      }
      for (int j = 0; j < N; ++j) {
        sem_wait(child_semaphore);  // Acquire lock
        shm_data->counter++;        // Critical section
        sem_post(child_semaphore);  // Release lock
      }
      sem_close(child_semaphore);  // Close semaphore descriptor in child
      munmap(shm_data, sizeof(shared_data_t));
      close(shm_fd);
      exit(EXIT_SUCCESS);  // Child exits
    }
  }

  for (int i = 0; i < K; ++i) {
    waitpid(pids[i], NULL, 0);  // Parent waits for children
  }

  gettimeofday(&end_time, NULL);  // End timing

  printf("Actual final value:   %lld\n", shm_data->counter);

  if (shm_data->counter != (long long)K * N) {
    printf("The value did not match. (Unexpected)\n");
  } else {
    printf("The value matched. (Expected)\n");
  }

  seconds = end_time.tv_sec - start_time.tv_sec;
  microseconds = end_time.tv_usec - start_time.tv_usec;
  elapsed_time_ms = (seconds * 1000.0) + (microseconds / 1000.0);
  printf("Execution time WITH named semaphore: %.3f ms\n", elapsed_time_ms);

  sem_close(semaphore);
  sem_unlink(SEM_NAME_NAMED_SEM);
  munmap(shm_data, sizeof(shared_data_t));
  close(shm_fd);
  shm_unlink(SHM_NAME_NAMED_SEM);

  return EXIT_SUCCESS;
}

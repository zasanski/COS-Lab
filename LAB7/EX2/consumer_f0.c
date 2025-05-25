#include <errno.h>     // For errno, ENOENT
#include <fcntl.h>     // For O_RDONLY
#include <math.h>      // For fabs
#include <stdio.h>     // For printf, perror
#include <stdlib.h>    // For EXIT_SUCCESS, EXIT_FAILURE
#include <sys/mman.h>  // For mmap, munmap
#include <unistd.h>    // For close, sleep

#include "shm_common.h"

#define F0_ESTIMATE_FREQ 1.0                   // Hz (once per second)
#define SAMPLES_PER_WINDOW (int)SAMPLING_FREQ  // Number of samples for a 1-second window

int main(void) {
  int shm_fd;
  circular_buffer_shm_t *shm_buffer;

  printf("--- Consumer K_F0 (PID %d) ---\n", getpid());

  // Open the shared memory segment (loop until available)
  while (1) {
    shm_fd = shm_open(SHM_NAME, O_RDONLY, 0);  // O_RDONLY for reading, mode 0
    if (shm_fd == -1) {
      if (errno == ENOENT) {
        fprintf(stderr, "K_F0: Shared memory '%s' not found. Retrying in 1 second...\n", SHM_NAME);
        sleep(1);
      } else {
        perror("shm_open failed unexpectedly");
        return EXIT_FAILURE;
      }
    } else {
      printf("K_F0: Shared memory segment '%s' opened (FD: %d).\n", SHM_NAME, shm_fd);
      break;
    }
  }

  // Map the shared memory segment into the process's address space
  shm_buffer = mmap(0, sizeof(circular_buffer_shm_t), PROT_READ, MAP_SHARED, shm_fd, 0);
  if (shm_buffer == MAP_FAILED) {
    perror("mmap failed");
    close(shm_fd);
    return EXIT_FAILURE;
  }
  printf("K_F0: Shared memory mapped to address %p.\n", (void *)shm_buffer);

  printf("K_F0: Estimating signal frequency (Ctrl+C to stop)...\n");

  // Loop to estimate frequency once per second
  while (1) {
    // Check if enough samples have been written by the producer
    if (shm_buffer->total_samples_written < SAMPLES_PER_WINDOW) {
      printf("K_F0: Not enough samples (currently %lld/%d). Waiting...\n", shm_buffer->total_samples_written,
             SAMPLES_PER_WINDOW);
      sleep(1);
      continue;  // Skip estimation until enough data is available
    }

    int current_write_pos = shm_buffer->write_pos;
    int zero_crossings = 0;
    float prev_sample;

    // Determine the starting position
    // Read SAMPLES_PER_WINDOW samples ending at (current_write_pos - 1)
    int start_read_pos = (current_write_pos - SAMPLES_PER_WINDOW + BUFFER_CAPACITY) % BUFFER_CAPACITY;

    // Get the first sample for comparison (sample before start_read_pos)
    int initial_compare_pos = (start_read_pos - 1 + BUFFER_CAPACITY) % BUFFER_CAPACITY;
    prev_sample = shm_buffer->samples[initial_compare_pos];

    // Iterate through the window to count zero crossings
    for (int i = 0; i < SAMPLES_PER_WINDOW; ++i) {
      int current_sample_pos = (start_read_pos + i) % BUFFER_CAPACITY;
      float current_sample = shm_buffer->samples[current_sample_pos];

      // Check for zero crossing (sign change)
      if ((prev_sample < 0 && current_sample >= 0) || (prev_sample > 0 && current_sample <= 0)) {
        zero_crossings++;
      }
      prev_sample = current_sample;
    }

    // Estimate frequency: (zero crossings / 2) / window_duration (1 second)
    float estimated_freq = (float)zero_crossings / 2.0f;

    printf("K_F0: Estimated Frequency: %.2f Hz (Zero Crossings: %d in %d samples)\n", estimated_freq, zero_crossings,
           SAMPLES_PER_WINDOW);

    // Sleep to estimate frequency at the desired frequency
    sleep(1);  // 1 second for 1Hz
  }

  // Consumers do not unlink the shared memory
  printf("K_F0: Cleaning up shared memory.\n");
  if (munmap(shm_buffer, sizeof(circular_buffer_shm_t)) == -1) {
    perror("munmap failed");
  }
  if (close(shm_fd) == -1) {
    perror("close shm_fd failed");
  }

  return EXIT_SUCCESS;
}

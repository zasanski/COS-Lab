#include <fcntl.h>     // For O_CREAT, O_RDWR
#include <math.h>      // For sin
#include <stdio.h>     // For printf, perror
#include <stdlib.h>    // For EXIT_SUCCESS, EXIT_FAILURE, srand, rand
#include <string.h>    // For memset
#include <sys/mman.h>  // For mmap, munmap
#include <time.h>      // For time
#include <unistd.h>    // For ftruncate, close, usleep

#include "shm_common.h"

int main(void) {
  int shm_fd;
  circular_buffer_shm_t *shm_buffer;
  double current_time = 0.0;  // Current time for sine wave calculation
  float sample_value;

  printf("--- Producer (PID %d) ---\n", getpid());

  //  Create the shared memory segment
  shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
  if (shm_fd == -1) {
    perror("shm_open failed");
    return EXIT_FAILURE;
  }
  printf("Producer: Shared memory segment '%s' created/opened (FD: %d).\n", SHM_NAME, shm_fd);

  // Set the size of the shared memory segment
  if (ftruncate(shm_fd, sizeof(circular_buffer_shm_t)) == -1) {
    perror("ftruncate failed");
    close(shm_fd);
    shm_unlink(SHM_NAME);
    return EXIT_FAILURE;
  }
  printf("Producer: Shared memory size set to %lu bytes.\n", sizeof(circular_buffer_shm_t));

  // Map the shared memory segment into the process's address space
  shm_buffer = mmap(0, sizeof(circular_buffer_shm_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (shm_buffer == MAP_FAILED) {
    perror("mmap failed");
    close(shm_fd);
    shm_unlink(SHM_NAME);
    return EXIT_FAILURE;
  }
  printf("Producer: Shared memory mapped to address %p.\n", (void *)shm_buffer);

  // Initialize the shared memory (especially write_pos and clear samples)
  shm_buffer->write_pos = 0;
  shm_buffer->total_samples_written = 0;
  memset(shm_buffer->samples, 0, sizeof(shm_buffer->samples));

  printf("Producer: Generating and writing samples...\n");

  // Loop continuously to generate and write samples
  while (1) {
    // Generate sinusoidal sample: Amplitude 1.0, SIGNAL_FREQ Hz
    sample_value = (float)sin(2 * PI * SIGNAL_FREQ * current_time);

    // Write sample and update sample count
    shm_buffer->samples[shm_buffer->write_pos] = sample_value;
    shm_buffer->total_samples_written++;
    // Update write position and wrap around
    shm_buffer->write_pos = (shm_buffer->write_pos + 1) % BUFFER_CAPACITY;

    // Update current time for next sample
    current_time += (1.0 / SAMPLING_FREQ);

    // Print status occasionally
    if (shm_buffer->total_samples_written % (long long)SAMPLING_FREQ == 0) {
      printf("Producer: Generated %lld samples. Latest: %.4f at pos %d\n", shm_buffer->total_samples_written,
             sample_value, shm_buffer->write_pos);
    }

    // Sleep for the inverse of the sampling frequency
    usleep(1000000 / SAMPLING_FREQ);
  }

  printf("Producer: Cleaning up shared memory.\n");
  if (munmap(shm_buffer, sizeof(circular_buffer_shm_t)) == -1) {
    perror("munmap failed");
  }
  if (close(shm_fd) == -1) {
    perror("close shm_fd failed");
  }
  if (shm_unlink(SHM_NAME) == -1) {
    perror("shm_unlink failed");
  }

  return EXIT_SUCCESS;
}

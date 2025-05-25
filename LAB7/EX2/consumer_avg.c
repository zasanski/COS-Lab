#include <errno.h>     // For errno, ENOENT
#include <fcntl.h>     // For O_RDONLY
#include <stdio.h>     // For printf, perror
#include <stdlib.h>    // For EXIT_SUCCESS, EXIT_FAILURE
#include <sys/mman.h>  // For mmap, munmap
#include <unistd.h>    // For close, usleep

#include "shm_common.h"

#define AVG_UPDATE_FREQ 10.0  // Hz (10 times per second)
#define ALPHA 0.1             // EMA smoothing factor

int main(void) {
  int shm_fd;
  circular_buffer_shm_t *shm_buffer;
  float ema = 0.0;
  int last_read_pos = -1;  // To track if we've read this sample before

  printf("--- Consumer K_AVG (PID %d) ---\n", getpid());

  // Open the shared memory segment (loop until available)
  while (1) {
    shm_fd = shm_open(SHM_NAME, O_RDONLY, 0);  // O_RDONLY for reading, mode 0
    if (shm_fd == -1) {
      if (errno == ENOENT) {
        fprintf(stderr, "K_AVG: Shared memory '%s' not found. Retrying in 1 second...\n", SHM_NAME);
        sleep(1);
      } else {
        perror("shm_open failed unexpectedly");
        return EXIT_FAILURE;
      }
    } else {
      printf("K_AVG: Shared memory segment '%s' opened (FD: %d).\n", SHM_NAME, shm_fd);
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
  printf("K_AVG: Shared memory mapped to address %p.\n", (void *)shm_buffer);

  printf("K_AVG: Monitoring Exponential Moving Average (EMA)...\n");

  // Loop to update EMA 10 times per second
  while (1) {
    // Get the current write position from the producer
    int current_write_pos = shm_buffer->write_pos;

    // Calculate the position of the latest written sample
    //(write_pos) points to the next free slot, so latest is at (write_pos - 1))
    int latest_sample_pos = (current_write_pos - 1 + BUFFER_CAPACITY) % BUFFER_CAPACITY;

    // Only update EMA if a new sample has been written since last check
    if (latest_sample_pos != last_read_pos) {
      float latest_sample = shm_buffer->samples[latest_sample_pos];

      // Initialize EMA with the first sample
      if (last_read_pos == -1) {
        ema = latest_sample;
      } else {
        // Update EMA: EMA_new = alpha * current_sample + (1 - alpha) * EMA_old
        ema = ALPHA * latest_sample + (1.0 - ALPHA) * ema;
      }
      printf("K_AVG: Latest Sample: %.4f at pos %d, Current EMA (alpha=%.2f): %.4f\n", latest_sample, latest_sample_pos,
             ALPHA, ema);
      last_read_pos = latest_sample_pos;
    } else {
      printf("K_AVG: No new sample yet. EMA: %.4f\n", ema);
    }

    // Sleep to update EMA at the desired frequency: 100ms for 10Hz
    usleep(1000000 / AVG_UPDATE_FREQ);
  }

  // Consumers do not unlink the shared memory
  printf("K_AVG: Cleaning up shared memory.\n");
  if (munmap(shm_buffer, sizeof(circular_buffer_shm_t)) == -1) {
    perror("munmap failed");
  }
  if (close(shm_fd) == -1) {
    perror("close shm_fd failed");
  }

  return EXIT_SUCCESS;
}

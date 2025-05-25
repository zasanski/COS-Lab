#ifndef SHM_COMMON_H
#define SHM_COMMON_H

#include <sys/types.h>
#include <unistd.h>

#define SHM_NAME "/circular_buffer_shm"
#define BUFFER_CAPACITY 4000  // Number of float samples in the circular buffer (2 seconds of data at 2kHz)
#define SAMPLING_FREQ 2000.0  // Hz (samples per second)
#define SIGNAL_FREQ 100.0     // Hz (frequency of the sine wave)
#define PI 3.14159265358979323846

// volatile: ensures compiler doesn't optimize away reads/writes
typedef struct {
  volatile int write_pos;                    // Current position where the producer writes the next sample
  volatile long long total_samples_written;  // Total number of samples ever written by producer
  float samples[BUFFER_CAPACITY];
} circular_buffer_shm_t;

#endif  // SHM_COMMON_H

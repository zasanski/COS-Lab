#include <errno.h>     // For errno, ENOENT
#include <fcntl.h>     // For O_RDWR
#include <stdio.h>     // For printf, perror
#include <stdlib.h>    // For EXIT_SUCCESS, EXIT_FAILURE
#include <string.h>    // For strncpy
#include <sys/mman.h>  // For mmap, munmap
#include <unistd.h>    // For close

#include "shm_common.h"

#define RETRY_DELAY_SEC 1  // Seconds to wait before retrying shm_open

int main(void) {
  int shm_fd;
  void *ptr;
  char buffer[SHM_SIZE];  // Local buffer to read into

  printf("--- Shared Memory Reader ---\n");

  // Open the shared memory segment
  // Loop until the shared memory is available (created by writer)
  while (1) {
    shm_fd = shm_open(SHM_NAME, O_RDONLY, 0);  // O_RDONLY for reading, mode 0
    if (shm_fd == -1) {
      if (errno == ENOENT) {
        // Shared memory does not exist
        fprintf(stderr, "Reader: Shared memory '%s' not found. Retrying in %d seconds...\n", SHM_NAME, RETRY_DELAY_SEC);
        sleep(RETRY_DELAY_SEC);
      } else {
        perror("shm_open failed unexpectedly");
        return EXIT_FAILURE;
      }
    } else {
      printf("Reader: Shared memory segment '%s' opened (FD: %d).\n", SHM_NAME, shm_fd);
      break;
    }
  }

  // Map the shared memory segment into the process's address space
  // PROT_READ: Memory can only be read from
  // MAP_SHARED: Changes are visible to other processes
  ptr = mmap(0, SHM_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
  if (ptr == MAP_FAILED) {
    perror("mmap failed");
    close(shm_fd);
    return EXIT_FAILURE;
  }
  printf("Reader: Shared memory mapped to address %p.\n", ptr);

  // Read data from the shared memory
  // Null-terminate the data if string as mmap provides raw memory.
  strncpy(buffer, (const char *)ptr, SHM_SIZE - 1);  // Copy string to local buffer
  buffer[SHM_SIZE - 1] = '\0';                       // Ensure null-termination
  printf("Reader: Read message: \"%s\"\n", buffer);

  // Unmap the shared memory from the process's address space
  if (munmap(ptr, SHM_SIZE) == -1) {
    perror("munmap failed");
  }
  printf("Reader: Shared memory unmapped.\n");

  // Close the shared memory file descriptor
  // The writer unlinks it right after writing.
  if (close(shm_fd) == -1) {
    perror("close shm_fd failed");
  }
  printf("Reader: Shared memory file descriptor closed.\n");

  printf("--- Reader Finished ---\n");
  return EXIT_SUCCESS;
}

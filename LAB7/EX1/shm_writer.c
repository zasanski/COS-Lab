#include <fcntl.h>     // For O_CREAT, O_RDWR
#include <stdio.h>     // For printf, perror
#include <stdlib.h>    // For EXIT_SUCCESS, EXIT_FAILURE
#include <string.h>    // For strcpy
#include <sys/mman.h>  // For mmap, munmap
#include <unistd.h>    // For ftruncate, close

#include "shm_common.h"

int main(void) {
  int shm_fd;
  void *ptr;
  const char *message = "Hello World!";

  printf("--- Shared Memory Writer ---\n");

  // Create the shared memory segment
  // O_CREAT: Create if it doesn't exist
  // O_RDWR: Open for reading and writing
  // 0666: Read/write permissions
  shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
  if (shm_fd == -1) {
    perror("shm_open failed");
    return EXIT_FAILURE;
  }
  printf("Writer: Shared memory segment '%s' created/opened (FD: %d).\n", SHM_NAME, shm_fd);

  // Set the size of the shared memory segment
  // As shm_open only creates/opens, it doesn't set size.
  if (ftruncate(shm_fd, SHM_SIZE) == -1) {
    perror("ftruncate failed");
    close(shm_fd);
    shm_unlink(SHM_NAME);  // Clean up if ftruncate fails
    return EXIT_FAILURE;
  }
  printf("Writer: Shared memory size set to %d bytes.\n", SHM_SIZE);

  // Map the shared memory segment into the process's address space
  // PROT_READ | PROT_WRITE: Memory can be read from and written to
  // MAP_SHARED: Changes are visible to other processes mapping the same memory
  // 0: Kernel chooses the address
  // 0: Offset from the beginning of the file
  ptr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (ptr == MAP_FAILED) {
    perror("mmap failed");
    close(shm_fd);
    shm_unlink(SHM_NAME);
    return EXIT_FAILURE;
  }
  printf("Writer: Shared memory mapped to address %p.\n", ptr);

  // Write data to the shared memory
  printf("Writer: Writing message: \"%s\"\n", message);
  strcpy((char *)ptr, message);

  // Sleep to ensure reader has time to read
  sleep(10);

  // Unmap the shared memory from the process's address space
  if (munmap(ptr, SHM_SIZE) == -1) {
    perror("munmap failed");
  }
  printf("Writer: Shared memory unmapped.\n");

  // Close the shared memory file descriptor
  if (close(shm_fd) == -1) {
    perror("close shm_fd failed");
  }
  printf("Writer: Shared memory file descriptor closed.\n");

  // Unlink it here so a fresh run can always create it.
  if (shm_unlink(SHM_NAME) == -1) {
    perror("shm_unlink failed");
  }
  printf("Writer: Shared memory segment '%s' unlinked.\n", SHM_NAME);

  printf("--- Writer Finished ---\n");
  return EXIT_SUCCESS;
}

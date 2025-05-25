#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void) {
  int pipefd[2];
  char write_buffer[] = "Hello, World!";
  char read_buffer[BUFSIZ];
  int bytes_written, bytes_read;

  // 1. Create a pipe
  if (pipe(pipefd) == -1) {
    perror("pipe");
    return EXIT_FAILURE;
  }

  // 2. Write data to the pipe
  bytes_written = write(pipefd[1], write_buffer, strlen(write_buffer) + 1);  // +1 for null terminator
  if (bytes_written == -1) {
    perror("write");
    close(pipefd[0]);  // Close read end
    close(pipefd[1]);  // Close write end
    return EXIT_FAILURE;
  }
  printf("Written %d bytes to pipe.\n", bytes_written);

  // 3. Read data from the pipe
  bytes_read = read(pipefd[0], read_buffer, BUFSIZ);
  if (bytes_read == -1) {
    perror("read");
    close(pipefd[0]);  // Close read end
    close(pipefd[1]);  // Close write end
    return EXIT_FAILURE;
  }
  printf("Read %d bytes from pipe.\n", bytes_read);

  // 4. Verify the data
  if (strcmp(write_buffer, read_buffer) == 0) {
    printf("Data verification successful: \"%s\"\n", read_buffer);
  } else {
    printf("Data verification failed.\n");
  }

  close(pipefd[0]);  // Close read end
  close(pipefd[1]);  // Close write end

  return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
  int pipefd[2];
  pid_t pid;
  char write_buffer[] = "Data from parent to child.";
  char read_buffer[BUFSIZ];
  int bytes_written, bytes_read;
  int status;

  // 1. Create a pipe
  if (pipe(pipefd) == -1) {
    perror("pipe");
    return EXIT_FAILURE;
  }

  // 2. Fork a child process
  pid = fork();
  if (pid == -1) {
    perror("fork");
    close(pipefd[0]);
    close(pipefd[1]);
    return EXIT_FAILURE;
  }

  if (pid == 0) {
    // Child process: read from the pipe and close the write end
    close(pipefd[1]);
    bytes_read = read(pipefd[0], read_buffer, BUFSIZ);
    if (bytes_read == -1) {
      perror("read");
      close(pipefd[0]);
      exit(EXIT_FAILURE);
    }
    printf("Child received: %s\n", read_buffer);
    close(pipefd[0]);
    exit(EXIT_SUCCESS);
  } else {
    // Parent process: write to the pipe close the read end
    close(pipefd[0]);
    bytes_written = write(pipefd[1], write_buffer, strlen(write_buffer) + 1);
    if (bytes_written == -1) {
      perror("write");
      close(pipefd[1]);
      return EXIT_FAILURE;
    }
    printf("Parent sent: %s\n", write_buffer);
    close(pipefd[1]);
    wait(&status);
    if (!WIFEXITED(status)) {
      printf("Child process did not terminate normally.\n");
      return EXIT_FAILURE;
    }
  }

  return 0;
}

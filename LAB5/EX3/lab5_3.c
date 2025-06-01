#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
  int pipefd[2];
  pid_t pid;
  char read_buffer[BUFSIZ];
  ssize_t bytes_read;
  int status;

  // Create a pipe
  if (pipe(pipefd) == -1) {
    perror("pipe");
    return EXIT_FAILURE;
  }

  // Fork a child process
  pid = fork();
  if (pid == -1) {
    perror("fork");
    close(pipefd[0]);
    close(pipefd[1]);
    return EXIT_FAILURE;
  }

  if (pid == 0) {
    // Child process
    close(pipefd[0]);  // Close the read end

    // Redirect stdout (file descriptor 1) to the write end
    if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
      perror("dup2");
      close(pipefd[1]);  // Close the write end
      exit(EXIT_FAILURE);
    }

    // Anything printed to stdout in the child will be written to the pipe
    printf("This message from the child will go through the pipe.\n");
    fprintf(stdout, "Another message from the child via fprintf.\n");
    fflush(stdout);    // Ensure the output is written to the pipe
    close(pipefd[1]);  // Close the write end after redirection
    exit(EXIT_SUCCESS);
  } else {
    // Parent process
    close(pipefd[1]);  // Close the write end

    // Read from the pipe until the child closes its write end
    printf("Parent reading from the pipe...\n");
    while ((bytes_read = read(pipefd[0], read_buffer, BUFSIZ - 1)) > 0) {
      read_buffer[bytes_read] = '\0';  // Null-terminate the received data
      printf("Parent received: %s", read_buffer);
    }

    if (bytes_read == -1) {
      perror("read");
    }

    close(pipefd[0]);  // Close the read end
    wait(&status);     // Wait for the child process to terminate
    if (WIFEXITED(status)) {
      printf("Child process exited normally.\n");
    } else {
      printf("Child process did not exit normally.\n");
    }
  }

  return 0;
}

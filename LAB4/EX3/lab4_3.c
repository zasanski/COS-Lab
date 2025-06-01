#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
  int parent_to_child[2];
  int child_to_parent[2];
  pid_t pid;
  char write_buffer[BUFSIZ];
  char read_buffer[BUFSIZ];
  int bytes_written, bytes_read;
  int status;

  // Create two pipes
  if (pipe(parent_to_child) == -1 || pipe(child_to_parent) == -1) {
    perror("pipe");
    return EXIT_FAILURE;
  }

  // Fork a child process
  pid = fork();
  if (pid == -1) {
    perror("fork");
    close(parent_to_child[0]);  // Close read end
    close(parent_to_child[1]);  // Close write end
    close(child_to_parent[0]);  // Close read end
    close(child_to_parent[1]);  // Close write end
    return EXIT_FAILURE;
  }

  if (pid == 0) {
    // Child process: echo server
    close(parent_to_child[1]);  // Close write end
    close(child_to_parent[0]);  // Close read end

    while (1) {
      bytes_read = read(parent_to_child[0], read_buffer, BUFSIZ);
      // Handle EOF or error
      if (bytes_read <= 0) {
        break;
      }

      // Null-terminate the string and convert to uppercase
      read_buffer[bytes_read] = '\0';
      for (int i = 0; i < bytes_read; i++) {
        read_buffer[i] = toupper(read_buffer[i]);
      }

      bytes_written = write(child_to_parent[1], read_buffer, bytes_read);
      if (bytes_written == -1) {
        perror("write");
        break;
      }
    }
    close(parent_to_child[0]);  // Close read end
    close(child_to_parent[1]);  // Close write end
    exit(EXIT_SUCCESS);
  } else {
    // Parent process: send and receive data
    close(parent_to_child[0]);  // Close read end
    close(child_to_parent[1]);  // Close write end

    char messages[][BUFSIZ] = {"hello", "hi", "heyyyyy", "hey"};
    int num_messages = sizeof(messages) / sizeof(messages[0]);

    for (int i = 0; i < num_messages; i++) {
      bytes_written = write(parent_to_child[1], messages[i], strlen(messages[i]) + 1);
      if (bytes_written == -1) {
        perror("write");
        break;
      }

      bytes_read = read(child_to_parent[0], read_buffer, BUFSIZ);
      if (bytes_read == -1) {
        perror("read");
        break;
      }
      read_buffer[bytes_read] = '\0';
      printf("Parent received: %s\n", read_buffer);
    }

    close(parent_to_child[1]);  // Close write end
    close(child_to_parent[0]);  // Close read end
    wait(&status);
    if (!WIFEXITED(status)) {
      printf("child did not exit normally\n");
    }
  }

  return 0;
}

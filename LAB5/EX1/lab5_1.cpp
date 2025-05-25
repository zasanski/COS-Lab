#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
  pid_t pid;
  int status;

  pid = fork();

  if (pid == -1) {
    perror("fork");
    return EXIT_FAILURE;
  }

  if (pid == 0) {
    // Child process: execute /usr/bin/cal
    execl("/usr/bin/cal", "cal", NULL);
    // Code after exec only executes on error
    perror("execl");
    exit(EXIT_FAILURE);
  } else {
    // Parent process: wait for child
    wait(&status);
    if (WIFEXITED(status)) {
      printf("Child process (cal) exited normally.\n");
    } else {
      printf("Child process (cal) did not exit normally.\n");
    }
  }

  return 0;
}

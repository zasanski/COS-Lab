#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
  pid_t pid;
  int status;

  pid = fork();
  switch (pid) {
    case -1:
      perror("fork");
      exit(EXIT_FAILURE);
    case 0:
      for (int i = 1; i <= 10; i++) {
        printf("Child: %d\n", i);
        sleep(1);
      }
      exit(EXIT_SUCCESS);
    default:
      wait(&status);
      if (WIFEXITED(status)) {
        printf("END OF WORK\n");
      } else {
        printf("Child process did not terminate normally\n");
      }
      exit(EXIT_SUCCESS);
  }
}

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

int main(void) {
  int a, b;
  pid_t pid;
  int status;

  // Generate random numbers
  srand(time(NULL));
  a = rand() % 11;       // 0 to 10
  b = rand() % 11 + 20;  // 20 to 30

  pid = fork();

  if (pid == -1) {
    perror("fork");
    exit(EXIT_FAILURE);
  } else if (pid == 0) {
    // Child process
    int sum = a + b;
    exit(sum);
  } else {
    wait(&status);
    if (WIFEXITED(status)) {
      int sum = WEXITSTATUS(status);
      printf("Sum of a (%d) and b (%d) is: %d\n", a, b, sum);
    } else {
      printf("Child process did not exit normally.\n");
    }
  }

  return 0;
}

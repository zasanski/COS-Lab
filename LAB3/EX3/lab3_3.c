#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define ARRAY_SIZE 1000000
#define NUM_CHILDREN 10
#define CHUNK_SIZE 100000

int main(void) {
  int *array;
  pid_t children[NUM_CHILDREN];
  int status;
  int partial_sums[NUM_CHILDREN];

  // Allocate memory for the array
  array = (int *)malloc(ARRAY_SIZE * sizeof(int));
  if (array == NULL) {
    perror("malloc");
    return EXIT_FAILURE;
  }

  // Fill the array with random values
  srand(time(NULL));
  for (int i = 0; i < ARRAY_SIZE; i++) {
    array[i] = rand() % 101;  // 0 to 100
  }

  // Create child processes
  for (int i = 0; i < NUM_CHILDREN; i++) {
    children[i] = fork();
    if (children[i] == -1) {
      perror("fork");
      return EXIT_FAILURE;
    } else if (children[i] == 0) {
      // Child process
      int start = i * CHUNK_SIZE;
      int end = start + CHUNK_SIZE;
      long long sum = 0;
      for (int j = start; j < end; j++) {
        sum += array[j];
      }
      int average = (int)((double)sum / CHUNK_SIZE);
      exit(average);
    }
  }

  // Parent process
  for (int i = 0; i < NUM_CHILDREN; i++) {
    wait(&status);
    if (WIFEXITED(status)) {
      partial_sums[i] = WEXITSTATUS(status);
    } else {
      printf("Child %d did not exit normally.\n", i);
      return EXIT_FAILURE;
    }
  }

  long long total_sum = 0;
  for (int i = 0; i < NUM_CHILDREN; i++) {
    total_sum += partial_sums[i];
  }

  int overall_average = (int)((double)total_sum / NUM_CHILDREN);
  printf("Overall average: %d\n", overall_average);
  free(array);

  return 0;
}

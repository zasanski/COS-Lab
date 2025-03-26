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
  float *array;
  pid_t children[NUM_CHILDREN];
  int pipes[NUM_CHILDREN][2];
  int status;
  float partial_sums[NUM_CHILDREN];

  // Allocate memory for the array
  array = (float *)malloc(ARRAY_SIZE * sizeof(float));
  if (array == NULL) {
    perror("malloc");
    return EXIT_FAILURE;
  }

  // Fill the array with random values (-1 to 1)
  srand(time(NULL));
  for (int i = 0; i < ARRAY_SIZE; i++) {
    array[i] = (float)rand() / RAND_MAX * 2.0f - 1.0f;
  }

  // Create child processes and pipes
  for (int i = 0; i < NUM_CHILDREN; i++) {
    if (pipe(pipes[i]) == -1) {
      perror("pipe");
      return EXIT_FAILURE;
    }

    children[i] = fork();
    if (children[i] == -1) {
      perror("fork");
      return EXIT_FAILURE;
    } else if (children[i] == 0) {
      // Child process
      close(pipes[i][0]);  // Close read end

      int start = i * CHUNK_SIZE;
      int end = start + CHUNK_SIZE;
      float sum = 0.0f;
      for (int j = start; j < end; j++) {
        sum += array[j];
      }

      // Write sum to pipe
      write(pipes[i][1], &sum, sizeof(float));
      close(pipes[i][1]);
      free(array);  // free allocated memory in the child process
      exit(EXIT_SUCCESS);
    } else {
      // Parent process
      close(pipes[i][1]);  // close write end
    }
  }

  // Parent process: wait for children and collect results
  for (int i = 0; i < NUM_CHILDREN; i++) {
    wait(&status);
    if (!WIFEXITED(status)) {
      printf("Child %d did not exit normally.\n", i);
      return EXIT_FAILURE;
    }

    // Read sum from pipe
    read(pipes[i][0], &partial_sums[i], sizeof(float));
    close(pipes[i][0]);
  }

  float total_sum = 0.0f;
  for (int i = 0; i < NUM_CHILDREN; i++) {
    total_sum += partial_sums[i];
  }

  float overall_average = total_sum / ARRAY_SIZE;
  printf("Overall average: %f\n", overall_average);
  free(array);

  return 0;
}

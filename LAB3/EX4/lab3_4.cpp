#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
  int status;
  float partial_sums[NUM_CHILDREN];
  char filename[20];
  FILE *file;

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
      float sum = 0.0f;
      for (int j = start; j < end; j++) {
        sum += array[j];
      }

      // Write sum to file
      sprintf(filename, "sum%d.txt", getpid());
      file = fopen(filename, "w");
      if (file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
      }
      fprintf(file, "%f", sum);
      fclose(file);
      exit(EXIT_SUCCESS);
    }
  }

  // Parent process
  for (int i = 0; i < NUM_CHILDREN; i++) {
    wait(&status);
    if (!WIFEXITED(status)) {
      printf("Child %d did not exit normally.\n", i);
      return EXIT_FAILURE;
    }
  }

  // Read sums from files and calculate average
  float total_sum = 0.0f;
  for (int i = 0; i < NUM_CHILDREN; i++) {
    sprintf(filename, "sum%d.txt", children[i]);
    file = fopen(filename, "r");
    if (file == NULL) {
      perror("fopen (read)");
      return EXIT_FAILURE;
    }
    fscanf(file, "%f", &partial_sums[i]);
    fclose(file);
    total_sum += partial_sums[i];
  }

  float overall_average = total_sum / ARRAY_SIZE;
  printf("Overall average: %f\n", overall_average);
  free(array);

  return 0;
}

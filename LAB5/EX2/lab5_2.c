#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
  int original_stdout_fd;
  int duplicated_stdout_fd;
  FILE *fake_stdout_file;
  char message1[] = "Writing to duplicated stdout using write() system call.\n";
  char message2[] = "Writing to duplicated stdout using fprintf() via fdopen().\n";

  // Duplicate the standard output file descriptor using dup()
  original_stdout_fd = fileno(stdout);
  duplicated_stdout_fd = dup(original_stdout_fd);
  if (duplicated_stdout_fd == -1) {
    perror("dup");
    return EXIT_FAILURE;
  }
  printf("Standard output file descriptor: %d\n", original_stdout_fd);
  printf("Duplicated stdout file descriptor: %d\n", duplicated_stdout_fd);

  // Write to the duplicated file descriptor using write()
  if (write(duplicated_stdout_fd, message1, sizeof(message1) - 1) == -1) {
    perror("write");
    close(duplicated_stdout_fd);
    return EXIT_FAILURE;
  }

  // Create a FILE stream from the duplicated file descriptor using fdopen()
  fake_stdout_file = fdopen(duplicated_stdout_fd, "w");
  if (fake_stdout_file == NULL) {
    perror("fdopen");
    close(duplicated_stdout_fd);
    return EXIT_FAILURE;
  }

  // Write to the FILE stream using fprintf()
  fprintf(fake_stdout_file, "%s", message2);

  // Close the FILE stream and the duplicated file descriptor
  fclose(fake_stdout_file);
  close(duplicated_stdout_fd);

  printf("This is printed using the default stdout.\n");
  fprintf(stdout, "This is also printed using the default stdout via fprintf.\n");

  return 0;
}

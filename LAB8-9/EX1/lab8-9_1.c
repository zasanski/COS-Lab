#include <pthread.h>  // For pthread_create, pthread_join
#include <stdio.h>    // For printf
#include <stdlib.h>   // For EXIT_SUCCESS, EXIT_FAILURE
#include <unistd.h>   // For sleep

// Function that will be executed by the new thread
void *thread_function(void *arg) {
  int start_value = *((int *)arg);  // Cast arg back to int pointer and dereference
  printf("Thread: Started. Will count from %d to %d.\n", start_value, start_value + 10);

  for (int i = start_value; i <= start_value + 10; ++i) {
    printf("Thread: Value = %d\n", i);
    sleep(1);
  }

  printf("Thread: Finished counting.\n");
  return NULL;
}

int main(void) {
  pthread_t my_thread_id;
  int initial_value = 0;

  printf("Main Thread: Starting.\n");

  // Create and start the new thread
  // &my_thread_id: Pointer to the pthread_t variable to store the new thread's ID.
  // NULL: Thread attributes (using default attributes).
  // thread_function: The function that the new thread will start executing.
  // (void *)&initial_value: Argument to pass to the thread_function.
  // Pass the address of initial_value, and cast it to void*.
  int rc = pthread_create(&my_thread_id, NULL, thread_function, (void *)&initial_value);
  if (rc != 0) {
    fprintf(stderr, "Main Thread: Error creating thread: %d\n", rc);
    return EXIT_FAILURE;
  }
  printf("Main Thread: New thread created with ID: %lu\n", (unsigned long)my_thread_id);

  sleep(4);
  printf("Main Thread: I did some work now I'm Waiting for the child thread to complete\n");

  // Wait for the created thread to complete
  // my_thread_id: The ID of the thread to wait for.
  // NULL: We are not interested in the thread's return value.
  rc = pthread_join(my_thread_id, NULL);
  if (rc != 0) {
    fprintf(stderr, "Main Thread: Error joining thread: %d\n", rc);
    return EXIT_FAILURE;
  }
  printf("Main Thread: Child thread completed so I exit.\n");

  return EXIT_SUCCESS;
}

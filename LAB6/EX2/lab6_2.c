#include <errno.h>   // For errno
#include <fcntl.h>   // For O_CREAT, O_EXCL, O_RDWR, O_WRONLY
#include <mqueue.h>  // For mq_open, mq_getattr, mq_setattr, mq_close, mq_unlink
#include <stdio.h>   // For printf, perror
#include <stdlib.h>  // For EXIT_SUCCESS, EXIT_FAILURE
#include <string.h>  // For strerror

#define QUEUE_NAME "/my_test_mq"
#define MAX_MSGS 5
#define MAX_MSG_SIZE 1024

void print_mq_attributes(mqd_t mq, const char *header) {
  struct mq_attr attr;
  if (mq_getattr(mq, &attr) == -1) {
    perror("mq_getattr failed");
    return;
  }
  printf("\n--- %s ---\n", header);
  printf("  mq_flags:   %ld (0 = blocking, O_NONBLOCK = non-blocking)\n", attr.mq_flags);
  printf("  mq_maxmsg:  %ld (Maximum # of messages on queue)\n", attr.mq_maxmsg);
  printf("  mq_msgsize: %ld (Maximum message size in bytes)\n", attr.mq_msgsize);
  printf("  mq_curmsgs: %ld (Number of messages currently in queue)\n", attr.mq_curmsgs);
}

int main(void) {
  mqd_t mq;
  struct mq_attr attr;
  struct mq_attr old_attr;

  // 1. Create a message queue with specific attributes
  printf("\nAttempting to create/open message queue '%s'...\n", QUEUE_NAME);
  attr.mq_flags = 0;               // Blocking queue
  attr.mq_maxmsg = MAX_MSGS;       // Max messages
  attr.mq_msgsize = MAX_MSG_SIZE;  // Max message size
  attr.mq_curmsgs = 0;             // Current messages

  // O_CREAT: Create the queue if it doesn't exist
  // O_EXCL: If O_CREAT is specified, and the queue already exists, return an error.
  // O_RDWR: Open for reading and writing
  // 0666: Read/write permissions
  mq = mq_open(QUEUE_NAME, O_CREAT | O_EXCL | O_RDWR, 0666, &attr);
  if (mq == (mqd_t)-1) {
    if (errno == EEXIST) {
      printf("Queue '%s' already exists. Opening it without O_EXCL.\n", QUEUE_NAME);
      mq = mq_open(QUEUE_NAME, O_RDWR, 0666, NULL);  // Open existing queue
      if (mq == (mqd_t)-1) {
        perror("mq_open failed when trying to open existing queue");
        return EXIT_FAILURE;
      }
      printf("Successfully opened existing queue.\n");
    } else {
      perror("mq_open failed during creation attempt");
      return EXIT_FAILURE;
    }
  } else {
    printf("Successfully created new queue '%s'.\n", QUEUE_NAME);
  }

  // 2. Check attributes
  print_mq_attributes(mq, "Current Queue Attributes");

  // 3. Try to change attributes
  printf("\nAttempting to change mq_flags to O_NONBLOCK...\n");
  struct mq_attr new_attr;
  new_attr.mq_flags = O_NONBLOCK;  // Set non-blocking flag

  // Note: Only mq_flags can generally be changed using mq_setattr after creation. Trying to change mq_maxmsg or
  // mq_msgsize via mq_setattr will usually fail with EINVAL or be ignored.
  if (mq_setattr(mq, &new_attr, &old_attr) == -1) {
    if (errno == EINVAL) {
      printf("Failed to set attributes: Invalid argument.\n");
    } else {
      perror("mq_setattr failed");
    }
  } else {
    printf("Successfully set mq_flags to O_NONBLOCK.\n");
    printf("Old mq_flags was: %ld\n", old_attr.mq_flags);
  }
  print_mq_attributes(mq, "Attributes After Attempted Change");

  // Try to set it back to blocking for demonstration
  printf("\nAttempting to change mq_flags back to blocking (0)...\n");
  new_attr.mq_flags = 0;
  if (mq_setattr(mq, &new_attr, &old_attr) == -1) {
    perror("mq_setattr failed when trying to set back to blocking");
  } else {
    printf("Successfully set mq_flags back to blocking. Old was: %ld\n", old_attr.mq_flags);
  }
  print_mq_attributes(mq, "Attributes After Setting Back to Blocking");

  // Check the difference between mq_close and mq_unlink
  printf("\n--- mq_close vs mq_unlink ---\n");
  printf("Calling mq_close(). This closes the descriptor for THIS process.\n");
  if (mq_close(mq) == -1) {
    perror("mq_close failed");
  } else {
    printf("mq_close successful.\n");
  }

  // Open it again after mq_close - it should work because it's not unlinked yet
  printf("Attempting to re-open queue after mq_close...\n");
  mq = mq_open(QUEUE_NAME, O_RDWR);
  if (mq == (mqd_t)-1) {
    perror("mq_open failed after mq_close");
  } else {
    printf("Successfully re-opened queue after mq_close.\n");
    print_mq_attributes(mq, "Attributes After Re-opening");
    if (mq_close(mq) == -1) {
      perror("mq_close failed on re-opened queue");
    } else {
      printf("mq_close successful on re-opened queue.\n");
    }
  }

  // The queue will only truly disappear when all processes have closed it AND mq_unlink has been called.
  printf("\nCalling mq_unlink(). This removes the queue from the system.\n");
  if (mq_unlink(QUEUE_NAME) == -1) {
    perror("mq_unlink failed");
    // If it fails, maybe it wasn't there or permissions
    return EXIT_FAILURE;
  } else {
    printf("mq_unlink successful. Queue '%s' is now marked for deletion.\n", QUEUE_NAME);
  }

  // Try to open it again after mq_unlink. It should fail as it's marked for deletion
  printf("Attempting to re-open queue after mq_unlink...\n");
  mq = mq_open(QUEUE_NAME, O_RDWR);
  if (mq == (mqd_t)-1) {
    perror("Expected: mq_open failed after mq_unlink");
  } else {
    // This might happen if another process still had it open or it was created again.
    printf("Unexpected: Successfully re-opened queue after mq_unlink. \n");
    mq_close(mq);           // Clean up if it somehow opened
    mq_unlink(QUEUE_NAME);  // Try to unlink again
  }

  return EXIT_SUCCESS;
}

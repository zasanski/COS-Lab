#include <errno.h>   // For errno
#include <fcntl.h>   // For O_RDONLY, O_WRONLY
#include <mqueue.h>  // For mq_open, mq_receive, mq_send, mq_close
#include <stdio.h>   // For printf, perror
#include <stdlib.h>  // For EXIT_SUCCESS, EXIT_FAILURE
#include <unistd.h>  // For getpid, sleep

#include "common.h"

#define RETRY_DELAY_SEC 1  // Seconds to wait before retrying mq_open

int main(void) {
  mqd_t task_mq;
  mqd_t result_mq;
  pid_t consumer_pid = getpid();

  printf("Slave (PID %d): Starting consumer.\n", consumer_pid);

  // Open Task Queue (for receiving tasks)
  // Open it in read-only mode, it must already exist (created by master)
  // Loop until the queue is available
  task_mq = mq_open(TASK_QUEUE_NAME, O_RDONLY);
  while (1) {
    task_mq = mq_open(TASK_QUEUE_NAME, O_RDONLY);
    if (task_mq == (mqd_t)-1) {
      if (errno == ENOENT) {
        fprintf(stderr, "Slave (PID %d): Task queue '%s' not found. Retrying in %d seconds...\n", consumer_pid,
                TASK_QUEUE_NAME, RETRY_DELAY_SEC);
        sleep(RETRY_DELAY_SEC);
      } else {
        perror("mq_open task_mq failed unexpectedly");
        return EXIT_FAILURE;
      }
    } else {
      printf("Slave (PID %d): Task queue '%s' opened successfully.\n", consumer_pid, TASK_QUEUE_NAME);
      break;
    }
  }

  // Open Result Queue (for sending results)
  // Open it in write-only mode, it must already exist (created by master)
  // Loop until the queue is available
  while (1) {
    result_mq = mq_open(RESULT_QUEUE_NAME, O_WRONLY);
    if (result_mq == (mqd_t)-1) {
      if (errno == ENOENT) {  // Queue does not exist
        fprintf(stderr, "Slave (PID %d): Result queue '%s' not found. Retrying in %d seconds...\n", consumer_pid,
                RESULT_QUEUE_NAME, RETRY_DELAY_SEC);
        sleep(RETRY_DELAY_SEC);
      } else {
        perror("mq_open result_mq failed unexpectedly");
        mq_close(task_mq);
        return EXIT_FAILURE;
      }
    } else {
      printf("Slave (PID %d): Result queue '%s' opened successfully.\n", consumer_pid, RESULT_QUEUE_NAME);
      break;
    }
  }

  // Infinite loop to process tasks
  printf("Slave (PID %d): Waiting for tasks...\n", consumer_pid);
  while (1) {
    task_t task;
    unsigned int prio;

    ssize_t bytes_read = mq_receive(task_mq, (char *)&task, sizeof(task_t), &prio);
    if (bytes_read == -1) {
      if (errno == EAGAIN) {
        // This would happen if mq_flags was O_NONBLOCK and no messages were available
        continue;
      }
      perror("mq_receive task failed");
      // A read error could mean the queue was unlinked by the master.
      break;
    } else if (bytes_read == 0) {
      // This generally means the queue was empty and then closed by all writers.
      // For POSIX MQs, a 0-byte read isn't typical for EOF like pipes.
      // mq_receive would block until a message is available or error.
      // If all producers have closed, mq_receive will continue to block.
      // The typical way for a consumer to know it's done is via a special "terminate" message.
      // For now, we'll rely on the master unlinking and then the slave
      // getting an error on subsequent mq_receive attempts or being killed.
    }

    // Simulate "difficult" work
    sleep(1);
    int sum = task.a + task.b;
    // Display results
    printf("    Slave PID: %d, Master PID: %d, Task ID %d: Result: %d + %d = %d\n", consumer_pid, task.producer_pid,
           task.task_id, task.a, task.b, sum);

    // Send result back to producer
    result_t result;
    result.consumer_pid = consumer_pid;
    result.producer_pid = task.producer_pid;
    result.task_id = task.task_id;
    result.result = sum;

    if (mq_send(result_mq, (const char *)&result, sizeof(result_t), MSG_PRIO) == -1) {
      perror("mq_send result failed");
      // This could happen if the result queue was closed/unlinked by the master prematurely
      break;  // Exit loop on send error
    }
  }

  // Cleanup
  // Slave does not unlink the queues. Only the master does that.
  if (mq_close(task_mq) == -1) {
    perror("mq_close task_mq failed");
  }
  if (mq_close(result_mq) == -1) {
    perror("mq_close result_mq failed");
  }

  return EXIT_SUCCESS;
}

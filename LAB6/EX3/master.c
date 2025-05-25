#include <fcntl.h>   // For O_CREAT, O_EXCL, O_RDWR etc.
#include <mqueue.h>  // For mq_open, mq_send, mq_receive, mq_close, mq_unlink
#include <stdio.h>   // For printf, perror
#include <stdlib.h>  // For atoi, EXIT_SUCCESS, EXIT_FAILURE
#include <time.h>    // For time, srand
#include <unistd.h>  // For getpid, sleep

#include "common.h"

int main(int argc, char *argv[]) {
  mqd_t task_mq;
  mqd_t result_mq;
  struct mq_attr attr;
  pid_t producer_pid = getpid();
  int num_tasks;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <number_of_tasks>\n", argv[0]);
    return EXIT_FAILURE;
  }
  num_tasks = atoi(argv[1]);
  if (num_tasks <= 0) {
    fprintf(stderr, "Number of tasks must be positive.\n");
    return EXIT_FAILURE;
  }

  // Seed random number generator using Use PID to make seeds more unique
  srand(time(NULL) ^ producer_pid);

  // Setup Task Queue (for sending tasks) and ensure message size is correct for task_t
  attr.mq_flags = 0;
  attr.mq_maxmsg = MAX_MSGS;
  attr.mq_msgsize = sizeof(task_t);
  attr.mq_curmsgs = 0;

  task_mq = mq_open(TASK_QUEUE_NAME, O_CREAT | O_WRONLY, 0666, &attr);
  if (task_mq == (mqd_t)-1) {
    perror("mq_open task_mq failed");
    return EXIT_FAILURE;
  }
  printf("Master (PID %d): Task queue '%s' opened/created successfully.\n", producer_pid, TASK_QUEUE_NAME);

  // Setup Result Queue (for receiving results). We use O_CREAT here, but we also ensure it's removed on exit.
  // The consumer might open it too, O_EXCL is not strictly needed for this.
  attr.mq_flags = 0;
  attr.mq_maxmsg = MAX_MSGS;
  attr.mq_msgsize = sizeof(result_t);
  attr.mq_curmsgs = 0;

  result_mq = mq_open(RESULT_QUEUE_NAME, O_CREAT | O_RDONLY, 0666, &attr);
  if (result_mq == (mqd_t)-1) {
    perror("mq_open result_mq failed");
    mq_close(task_mq);           // Clean up
    mq_unlink(TASK_QUEUE_NAME);  // Clean up in case we created it
    return EXIT_FAILURE;
  }
  printf("Master (PID %d): Result queue '%s' opened/created successfully.\n", producer_pid, RESULT_QUEUE_NAME);

  // Send tasks
  printf("Master (PID %d): Generating and sending %d tasks...\n", producer_pid, num_tasks);
  for (int i = 0; i < num_tasks; ++i) {
    task_t task;
    task.producer_pid = producer_pid;
    task.task_id = i + 1;
    task.a = rand() % 100;  // Random A (0-99)
    task.b = rand() % 100;  // Random B (0-99)

    if (mq_send(task_mq, (const char *)&task, sizeof(task_t), MSG_PRIO) == -1) {
      perror("mq_send task failed");
    } else {
      printf("  Master PID: %d, Task ID: %d, A=%d, B=%d\n", producer_pid, task.task_id, task.a, task.b);
    }
    // Small delay to simulate tasks being generated over time
    usleep(100000);  // 100ms
  }

  // Receive results
  printf("Master (PID %d): Waiting for %d results...\n", producer_pid, num_tasks);
  for (int i = 0; i < num_tasks; ++i) {
    result_t result;
    unsigned int prio;
    if (mq_receive(result_mq, (char *)&result, sizeof(result_t), &prio) == -1) {
      perror("mq_receive result failed");
      break;
    } else {
      printf("  Master PID: %d, Slave PID: %d, Task ID: %d, Result: %d\n", producer_pid, result.consumer_pid,
             result.task_id, result.result);
    }
  }
  printf("Master (PID %d): All tasks sent and results received.\n", producer_pid);

  // Cleanup
  if (mq_close(task_mq) == -1) {
    perror("mq_close task_mq failed");
  }
  if (mq_unlink(TASK_QUEUE_NAME) == -1) {
    perror("mq_unlink task_queue failed");
  }
  if (mq_close(result_mq) == -1) {
    perror("mq_close result_mq failed");
  }
  if (mq_unlink(RESULT_QUEUE_NAME) == -1) {
    perror("mq_unlink result_queue failed");
  }

  return EXIT_SUCCESS;
}

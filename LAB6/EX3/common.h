#ifndef COMMON_H
#define COMMON_H

#include <sys/types.h>

#define TASK_QUEUE_NAME "/task_queue"
#define RESULT_QUEUE_NAME "/result_queue"

#define MAX_MSGS 10       // Max messages in queue
#define MAX_MSG_SIZE 256  // Max size of a message
#define MSG_PRIO 1        // Message priority

// Structure for a task
typedef struct {
  pid_t producer_pid;
  int task_id;
  int a;
  int b;
} task_t;

// Structure for a result
typedef struct {
  pid_t consumer_pid;
  pid_t producer_pid;
  int task_id;
  int result;
} result_t;

#endif  // COMMON_H

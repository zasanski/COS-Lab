#ifndef SHM_COUNTER_COMMON_H
#define SHM_COUNTER_COMMON_H

#include <semaphore.h>

// Unsynced
#define SHM_NAME_UNSYNCED "/shm_unsynced_counter_lab8_3"

// Named semaphore
#define SEM_NAME_NAMED_SEM "/sem_named_counter_lab8_3"
#define SHM_NAME_NAMED_SEM "/shm_named_sem_counter_lab8_3"

// Unnamed semaphore
#define SHM_NAME_UNNAMED_SEM "/shm_unnamed_sem_counter_lab8_3"

// Structure for the data stored in shared memory
// sem_t mutex for the unnamed semaphore case. For other cases this field will simply be ignored.
typedef struct {
  sem_t mutex;  // The anonymous semaphore itself must be in shared memory for unnamed
  long long counter;
} shared_data_t;

#endif  // SHM_COUNTER_COMMON_H

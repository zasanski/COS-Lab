# LAB10-11/EX1/CMakeLists.txt

# Unsynced
add_executable(shm_counter_unsynced shm_counter_unsynced.c shm_counter_common.h)
target_link_libraries(shm_counter_unsynced rt)

# Named semaphore
add_executable(shm_counter_named_sem shm_counter_named_sem.c shm_counter_common.h)
target_link_libraries(shm_counter_named_sem rt)

# Unnamed semaphore
add_executable(shm_counter_unnamed_sem shm_counter_unnamed_sem.c shm_counter_common.h)
target_link_libraries(shm_counter_unnamed_sem rt)

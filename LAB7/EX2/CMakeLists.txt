# LAB7/EX2/CMakeLists.txt

add_executable(producer producer.c shm_common.h)
target_link_libraries(producer rt m)

add_executable(consumer_avg consumer_avg.c shm_common.h)
target_link_libraries(consumer_avg rt m)

add_executable(consumer_f0 consumer_f0.c shm_common.h)
target_link_libraries(consumer_f0 rt m)

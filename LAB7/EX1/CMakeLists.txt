# LAB7/EX1/CMakeLists.txt

add_executable(shm_writer shm_writer.c shm_common.h)
target_link_libraries(shm_writer rt)

add_executable(shm_reader shm_reader.c shm_common.h)
target_link_libraries(shm_reader rt)

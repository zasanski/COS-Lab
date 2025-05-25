# LAB6/EX3/CMakeLists.txt

add_executable(master master.c common.h)
target_link_libraries(master rt)

add_executable(slave slave.c common.h)
target_link_libraries(slave rt)

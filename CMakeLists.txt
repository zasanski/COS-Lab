cmake_minimum_required(VERSION 3.28)
project(COS)

# Enable C++ language support
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the default build type to Debug
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# Set the output directory for all executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

# Add subdirectories for each lab
add_subdirectory(LAB1)
add_subdirectory(LAB2)
add_subdirectory(LAB3)
add_subdirectory(LAB4)
add_subdirectory(LAB5)
add_subdirectory(LAB6)
add_subdirectory(LAB7)
add_subdirectory(LAB8-9)
add_subdirectory(LAB10-11)
add_subdirectory(LAB12)

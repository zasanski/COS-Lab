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

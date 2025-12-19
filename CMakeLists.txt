cmake_minimum_required(VERSION 3.5)

project(LearningLinuxKernel LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_subdirectory(Applications)

# the kernel modules should only compile on Linux!
if (NOT WIN32 AND NOT APPLE)
    add_subdirectory(KernelModules)
endif()

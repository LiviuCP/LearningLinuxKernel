cmake_minimum_required(VERSION 3.5)

project(LearningLinuxKernel LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_subdirectory(Applications)

add_custom_target(KernelModules)

set(KERNEL_MODULES_SOURCE_DIR ${CMAKE_SOURCE_DIR}/KernelModules)
set(KERNEL_MODULES_BUILD_DIR ${CMAKE_BINARY_DIR}/KernelModules)

execute_process(COMMAND bash -c "cd ${KERNEL_MODULES_SOURCE_DIR}/MinMax && make MY_DIR=${KERNEL_MODULES_BUILD_DIR}/MinMax")

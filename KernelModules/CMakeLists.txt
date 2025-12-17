cmake_minimum_required(VERSION 3.5)

project(KernelModules LANGUAGES C)

set(KERNEL_MODULES_SOURCE_DIR ${CMAKE_SOURCE_DIR}/KernelModules)
set(KERNEL_MODULES_BUILD_DIR ${CMAKE_BINARY_DIR}/KernelModules)

set(KERNEL_MODULES_CONSOLIDATED_OUTPUT_DIR ${KERNEL_MODULES_BUILD_DIR}/ConsolidatedOutput)

add_subdirectory(Average)
add_subdirectory(Division)
add_subdirectory(MinMax)

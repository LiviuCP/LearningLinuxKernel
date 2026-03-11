cmake_minimum_required(VERSION 3.5)

project(IoctlStringOps LANGUAGES C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(${KERNEL_MODULES_SOURCE_DIR}/CMakeUtils/customtarget.cmake)
include(${KERNEL_MODULES_SOURCE_DIR}/CMakeUtils/customcleantarget.cmake)

target_sources(${PROJECT_NAME} PRIVATE ioctl_string_ops.c Makefile)

copy_file_to_consolidated_output(ioctl_string_ops.ko)

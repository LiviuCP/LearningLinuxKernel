cmake_minimum_required(VERSION 3.5)

project(IoctlDivision LANGUAGES C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(${KERNEL_MODULES_SOURCE_DIR}/CMakeUtils/customtarget.cmake)
include(${KERNEL_MODULES_SOURCE_DIR}/CMakeUtils/customcleantarget.cmake)

target_sources(${PROJECT_NAME} PRIVATE
    include/ioctl_division_impl.h
    source/ioctl_division_impl.c
    source/ioctl_division_main.c
    Makefile)

copy_file_to_consolidated_output(ioctl_division.ko)

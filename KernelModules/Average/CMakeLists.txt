cmake_minimum_required(VERSION 3.5)

project(Average LANGUAGES C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(${KERNEL_MODULES_SOURCE_DIR}/CMakeUtils/customtarget.cmake)
include(${KERNEL_MODULES_SOURCE_DIR}/CMakeUtils/customcleantarget.cmake)

target_sources(${PROJECT_NAME} PRIVATE
    include/Common/average_utilities.h
    source/Common/average_exit.c
    source/Common/average_utilities.c
    source/V1/average_init.c
    source/V2/average_init.c
    Makefile)

copy_file_to_consolidated_output(average1.ko)
copy_file_to_consolidated_output(average2.ko)

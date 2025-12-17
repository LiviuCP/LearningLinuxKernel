cmake_minimum_required(VERSION 3.5)

project(Average LANGUAGES C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(${KERNEL_MODULES_SOURCE_DIR}/CMakeUtils/customtarget.cmake)

target_sources(${PROJECT_NAME} PRIVATE
    source/Common/average_exit.c
    source/Common/average_print.c
    source/Common/average_utilities.c
    source/V1/average_init.c
    source/V2/average_init.c
    Makefile)

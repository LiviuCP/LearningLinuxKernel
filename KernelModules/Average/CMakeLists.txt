cmake_minimum_required(VERSION 3.5)

project(Average LANGUAGES C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(${KERNEL_MODULES_SOURCE_DIR}/CMakeUtils/customtarget.cmake)

target_sources(${PROJECT_NAME} PRIVATE
    average1_init.c
    average2_init.c
    average_exit.c
    common.c
    utilities.c
    Makefile)

cmake_minimum_required(VERSION 3.5)

project(Division LANGUAGES C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(${KERNEL_MODULES_SOURCE_DIR}/CMakeUtils/customtarget.cmake)

target_sources(${PROJECT_NAME} PRIVATE
    include/division_impl.h
    source/division_impl.c
    source/division_main.c
    Makefile)

copy_file_to_consolidated_output(division.ko)

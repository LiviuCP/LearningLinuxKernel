cmake_minimum_required(VERSION 3.5)

project(Division LANGUAGES C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(${KERNEL_MODULES_SOURCE_DIR}/CMakeUtils/customtarget.cmake)

target_sources(${PROJECT_NAME} PRIVATE
    division_main.c
    division_impl.c
    include/division_impl.h
    Makefile)

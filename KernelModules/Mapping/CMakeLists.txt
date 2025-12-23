cmake_minimum_required(VERSION 3.5)

project(Mapping LANGUAGES C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(${KERNEL_MODULES_SOURCE_DIR}/CMakeUtils/customtarget.cmake)
include(${KERNEL_MODULES_SOURCE_DIR}/CMakeUtils/customcleantarget.cmake)

target_sources(${PROJECT_NAME} PRIVATE
    source/mapping_main.c
    Makefile)

copy_file_to_consolidated_output(mapping.ko)

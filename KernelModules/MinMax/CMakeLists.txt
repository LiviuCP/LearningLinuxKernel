cmake_minimum_required(VERSION 3.5)

project(MinMax LANGUAGES C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(${KERNEL_MODULES_SOURCE_DIR}/CMakeUtils/customtarget.cmake)

target_sources(${PROJECT_NAME} PRIVATE minMax.c Makefile)

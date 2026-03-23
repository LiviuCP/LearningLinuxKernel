project(Division LANGUAGES C)

include(${KERNEL_MODULES_SOURCE_DIR}/CMakeUtils/customtarget.cmake)
include(${KERNEL_MODULES_SOURCE_DIR}/CMakeUtils/customcleantarget.cmake)

target_sources(${PROJECT_NAME} PRIVATE
    include/division_impl.h
    source/division_impl.c
    source/division_main.c
    Makefile)

copy_file_to_consolidated_output(division.ko)

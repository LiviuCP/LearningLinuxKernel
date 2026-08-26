project(ProcfsDivision LANGUAGES C)

include(${KERNEL_MODULES_SOURCE_DIR}/CMakeUtils/customtarget.cmake)
include(${KERNEL_MODULES_SOURCE_DIR}/CMakeUtils/customcleantarget.cmake)

target_sources(${PROJECT_NAME} PRIVATE
    include/procfs_division_impl.h
    source/procfs_division_impl.c
    source/procfs_division_main.c
    Makefile)

copy_file_to_consolidated_output(procfs_division.ko)

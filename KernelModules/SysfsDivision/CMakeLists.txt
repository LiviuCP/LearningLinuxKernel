project(SysfsDivision LANGUAGES C)

include(${KERNEL_MODULES_SOURCE_DIR}/CMakeUtils/customtarget.cmake)
include(${KERNEL_MODULES_SOURCE_DIR}/CMakeUtils/customcleantarget.cmake)

target_sources(${PROJECT_NAME} PRIVATE
    include/sysfs_division_impl.h
    source/sysfs_division_impl.c
    source/sysfs_division_main.c
    Makefile)

copy_file_to_consolidated_output(sysfs_division.ko)

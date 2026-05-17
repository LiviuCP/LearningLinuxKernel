project(StringOps LANGUAGES C)

include(${KERNEL_MODULES_SOURCE_DIR}/CMakeUtils/customtarget.cmake)
include(${KERNEL_MODULES_SOURCE_DIR}/CMakeUtils/customcleantarget.cmake)

target_sources(${PROJECT_NAME} PRIVATE
    include/string_ops_impl.h
    source/string_ops_impl.c
    source/string_ops_main.c
    Makefile)

copy_file_to_consolidated_output(string_ops.ko)

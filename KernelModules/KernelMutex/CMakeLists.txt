project(KernelMutex LANGUAGES C)

include(${KERNEL_MODULES_SOURCE_DIR}/CMakeUtils/customtarget.cmake)
include(${KERNEL_MODULES_SOURCE_DIR}/CMakeUtils/customcleantarget.cmake)

target_sources(${PROJECT_NAME} PRIVATE
    include/kernel_mutex_impl.h
    source/kernel_mutex_impl.c
    source/kernel_mutex_main.c
    Makefile)

copy_file_to_consolidated_output(kernel_mutex.ko)

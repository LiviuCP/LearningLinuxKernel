project(WaitQueue LANGUAGES C)

include(${KERNEL_MODULES_SOURCE_DIR}/CMakeUtils/customtarget.cmake)
include(${KERNEL_MODULES_SOURCE_DIR}/CMakeUtils/customcleantarget.cmake)

target_sources(${PROJECT_NAME} PRIVATE
    include/wait_queue_impl.h
    source/wait_queue_impl.c
    source/wait_queue_main.c
    Makefile)

copy_file_to_consolidated_output(wait_queue.ko)

project(TimeoutSignaler LANGUAGES C)

include(${KERNEL_MODULES_SOURCE_DIR}/CMakeUtils/customtarget.cmake)
include(${KERNEL_MODULES_SOURCE_DIR}/CMakeUtils/customcleantarget.cmake)

target_sources(${PROJECT_NAME} PRIVATE
    include/timeout_signaler_impl.h
    source/timeout_signaler_impl.c
    source/timeout_signaler_main.c
    Makefile)

copy_file_to_consolidated_output(timeout_signaler.ko)

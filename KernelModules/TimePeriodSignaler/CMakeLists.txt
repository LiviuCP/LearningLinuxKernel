project(TimePeriodSignaler LANGUAGES C)

include(${KERNEL_MODULES_SOURCE_DIR}/CMakeUtils/customtarget.cmake)
include(${KERNEL_MODULES_SOURCE_DIR}/CMakeUtils/customcleantarget.cmake)

target_sources(${PROJECT_NAME} PRIVATE
    include/time_period_signaler_impl.h
    source/time_period_signaler_impl.c
    source/time_period_signaler_main.c
    Makefile)

copy_file_to_consolidated_output(time_period_signaler.ko)

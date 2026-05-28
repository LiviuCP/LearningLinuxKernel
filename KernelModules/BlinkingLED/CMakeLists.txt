project(BlinkingLED LANGUAGES C)

include(${KERNEL_MODULES_SOURCE_DIR}/CMakeUtils/customtarget.cmake)
include(${KERNEL_MODULES_SOURCE_DIR}/CMakeUtils/customcleantarget.cmake)

target_sources(${PROJECT_NAME} PRIVATE
    include/blinking_led_impl.h
    source/blinking_led_impl.c
    source/blinking_led_main.c
    Makefile)

copy_file_to_consolidated_output(blinking_led.ko)

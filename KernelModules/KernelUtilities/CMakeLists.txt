cmake_minimum_required(VERSION 3.5)

add_custom_target(KernelUtils)

# one .h file to be created per function in order to avoid loading unnecessary functions into the kernel module
target_sources(KernelUtils PRIVATE trim_and_copy_string.h)

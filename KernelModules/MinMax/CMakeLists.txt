cmake_minimum_required(VERSION 3.5)

project(MinMax LANGUAGES C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_custom_target(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE minMax.c)

execute_process(COMMAND bash -c "cd ${KERNEL_MODULES_SOURCE_DIR}/${PROJECT_NAME} && make BUILD_DIR=${KERNEL_MODULES_BUILD_DIR}/${PROJECT_NAME}")

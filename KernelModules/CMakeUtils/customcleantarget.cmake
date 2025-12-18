# set building the custom clean target ([project_name]_clean) as build step after the clean step in the project configuration
# example:
# - "regular" clean: cmake --build /home/Liviu/bin/Builds/LearningLinuxKernel --target clean
# - followed by "custom" clean: cmake --build /home/Liviu/bin/Builds/LearningLinuxKernel --target Division_clean Average_clean MinMax_clean

add_custom_target("${PROJECT_NAME}_clean"
    COMMAND ${CMAKE_COMMAND}
    -DKERNEL_MODULES_SRC_DIR=${KERNEL_MODULES_SOURCE_DIR}
    -DKERNEL_MODULES_BUILD_DIR=${KERNEL_MODULES_BUILD_DIR}
    -DPROJECT_NAME=${PROJECT_NAME}
    -P ${KERNEL_MODULES_SOURCE_DIR}/CMakeUtils/makeclean.cmake
)

add_dependencies(${KERNEL_MODULES_PROJECT_NAME}_clean "${PROJECT_NAME}_clean")

project(KernelModules LANGUAGES C)

set(KERNEL_MODULES_PROJECT_NAME ${PROJECT_NAME})
set(KERNEL_MODULES_SOURCE_DIR ${CMAKE_SOURCE_DIR}/${KERNEL_MODULES_PROJECT_NAME})
set(KERNEL_MODULES_BUILD_DIR ${CMAKE_BINARY_DIR}/${KERNEL_MODULES_PROJECT_NAME})
set(KERNEL_MODULES_CONSOLIDATED_OUTPUT_DIR ${KERNEL_MODULES_BUILD_DIR}/ConsolidatedOutput)

add_custom_target(${KERNEL_MODULES_PROJECT_NAME}_clean)

add_subdirectory(Average)
add_subdirectory(Division)
add_subdirectory(IoctlDivision)
add_subdirectory(IoctlStringOps)
add_subdirectory(Mapping)
add_subdirectory(MinMax)
add_subdirectory(StringOps)
add_subdirectory(KernelUtilities)

add_dependencies(Division KernelUtilities)
add_dependencies(IoctlDivision KernelUtilities)
add_dependencies(IoctlStringOps KernelUtilities)
add_dependencies(Mapping KernelUtilities)
add_dependencies(StringOps KernelUtilities)

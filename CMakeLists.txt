cmake_minimum_required(VERSION 3.5)

project(LearningLinuxKernel LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# this variable is propagated to all nested libraries (should be declared BEFORE any subdir)
set(LIB_TYPE SHARED)

add_subdirectory(Applications)

# the kernel modules should only compile on Linux!
if (NOT WIN32 AND NOT APPLE)
    add_subdirectory(KernelModules)
endif()

find_package(QT NAMES Qt5 Qt6 COMPONENTS Test QUIET)

if(QT_FOUND)
    add_subdirectory(Tests)
endif()

add_subdirectory(Utilities)

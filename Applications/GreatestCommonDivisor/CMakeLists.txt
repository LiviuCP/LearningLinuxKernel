cmake_minimum_required(VERSION 3.5)

project(GreatestCommonDivisor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(
    Common
    IOCTL_GCD
    SYSFS_GCD
    ../../Utilities
)

add_executable(Ioctl${PROJECT_NAME}
    ioctlgcdmain.cpp
    Common/gcdloaderimpl.cpp
    Common/gcdparser.cpp
    IOCTL_GCD/ioctlgcdcore.cpp
    IOCTL_GCD/ioctlgcdloader.cpp
)

add_executable(Sysfs${PROJECT_NAME}
    sysfsgcdmain.cpp
    Common/gcdloaderimpl.cpp
    Common/gcdparser.cpp
    SYSFS_GCD/sysfsgcdcore.cpp
    SYSFS_GCD/sysfsgcdloader.cpp
)

target_link_libraries(Ioctl${PROJECT_NAME} PRIVATE Utilities)
target_link_libraries(Sysfs${PROJECT_NAME} PRIVATE Utilities)

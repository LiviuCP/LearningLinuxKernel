project(GreatestCommonDivisor LANGUAGES CXX)

include_directories(
    Common
    IOCTL_GCD
    PROCFS_GCD
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

add_executable(Procfs${PROJECT_NAME}
    procfsgcdmain.cpp
    Common/gcdloaderimpl.cpp
    Common/gcdparser.cpp
    PROCFS_GCD/procfsgcdcore.cpp
    PROCFS_GCD/procfsgcdloader.cpp
)

add_executable(Sysfs${PROJECT_NAME}
    sysfsgcdmain.cpp
    Common/gcdloaderimpl.cpp
    Common/gcdparser.cpp
    SYSFS_GCD/sysfsgcdcore.cpp
    SYSFS_GCD/sysfsgcdloader.cpp
)

target_link_libraries(Ioctl${PROJECT_NAME} PRIVATE Utilities)
target_link_libraries(Procfs${PROJECT_NAME} PRIVATE Utilities)
target_link_libraries(Sysfs${PROJECT_NAME} PRIVATE Utilities)

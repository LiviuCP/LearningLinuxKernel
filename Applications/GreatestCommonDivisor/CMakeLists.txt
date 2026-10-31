project(GreatestCommonDivisor LANGUAGES CXX)

include_directories(
    Common
    IOCTL_GCD
    PROCFS_SYSFS_GCD
    ../../Utilities
)

add_executable(Ioctl${PROJECT_NAME}
    gcdmain.cpp
    Common/gcdapp.cpp
    Common/gcdcore_common.cpp
    Common/gcdloader_common.cpp
    Common/gcdparser.cpp
    IOCTL_GCD/gcdcore.cpp
    IOCTL_GCD/gcdloader.cpp
)

add_executable(Procfs${PROJECT_NAME}
    gcdmain.cpp
    Common/gcdapp.cpp
    Common/gcdcore_common.cpp
    Common/gcdloader_common.cpp
    Common/gcdparser.cpp
    PROCFS_SYSFS_GCD/gcdcore.cpp
    PROCFS_SYSFS_GCD/gcdloader.cpp
)

add_executable(Sysfs${PROJECT_NAME}
    gcdmain.cpp
    Common/gcdapp.cpp
    Common/gcdcore_common.cpp
    Common/gcdloader_common.cpp
    Common/gcdparser.cpp
    PROCFS_SYSFS_GCD/gcdcore.cpp
    PROCFS_SYSFS_GCD/gcdloader.cpp
)

target_link_libraries(Ioctl${PROJECT_NAME} PRIVATE Utilities)
target_link_libraries(Procfs${PROJECT_NAME} PRIVATE Utilities)
target_link_libraries(Sysfs${PROJECT_NAME} PRIVATE Utilities)

target_compile_definitions(Ioctl${PROJECT_NAME} PRIVATE IOCTL_GCD)
target_compile_definitions(Procfs${PROJECT_NAME} PRIVATE PROCFS_GCD)
target_compile_definitions(Sysfs${PROJECT_NAME} PRIVATE SYSFS_GCD)

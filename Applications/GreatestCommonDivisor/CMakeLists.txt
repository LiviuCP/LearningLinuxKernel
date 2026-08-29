project(GreatestCommonDivisor LANGUAGES CXX)

include_directories(
    Common
    IOCTL_GCD
    PROCFS_SYSFS_GCD
    ../../Utilities
)

add_executable(Ioctl${PROJECT_NAME}
    gcdmain.cpp
    Common/gcdloader.cpp
    Common/gcdparser.cpp
    IOCTL_GCD/gcdapp.cpp
    IOCTL_GCD/gcdcore.cpp
)

add_executable(Procfs${PROJECT_NAME}
    gcdmain.cpp
    Common/gcdloader.cpp
    Common/gcdparser.cpp
    PROCFS_SYSFS_GCD/gcdapp.cpp
    PROCFS_SYSFS_GCD/gcdcore.cpp
)

add_executable(Sysfs${PROJECT_NAME}
    gcdmain.cpp
    Common/gcdloader.cpp
    Common/gcdparser.cpp
    PROCFS_SYSFS_GCD/gcdapp.cpp
    PROCFS_SYSFS_GCD/gcdcore.cpp
)

target_link_libraries(Ioctl${PROJECT_NAME} PRIVATE Utilities)
target_link_libraries(Procfs${PROJECT_NAME} PRIVATE Utilities)
target_link_libraries(Sysfs${PROJECT_NAME} PRIVATE Utilities)

target_compile_definitions(Ioctl${PROJECT_NAME} PRIVATE IOCTL_GCD)
target_compile_definitions(Procfs${PROJECT_NAME} PRIVATE PROCFS_GCD)
target_compile_definitions(Sysfs${PROJECT_NAME} PRIVATE SYSFS_GCD)

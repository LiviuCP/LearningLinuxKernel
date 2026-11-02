project(GreatestCommonDivisor LANGUAGES CXX)

include_directories(
    Common
    Impl/include
    IOCTL_GCD
    PROCFS_SYSFS_GCD
    ../../Utilities
)

add_executable(Ioctl${PROJECT_NAME}
    gcdmain.cpp
    Common/gcdapp.cpp
    Common/gcdcore.cpp
    Common/gcdloader.cpp
    Common/gcdparser.cpp
    Impl/IOCTL/gcdcore_impl.cpp
    Impl/IOCTL/gcdloader_impl.cpp
)

add_executable(Procfs${PROJECT_NAME}
    gcdmain.cpp
    Common/gcdapp.cpp
    Common/gcdcore.cpp
    Common/gcdloader.cpp
    Common/gcdparser.cpp
    Impl/PROCFS_SYSFS/gcdcore_impl.cpp
    Impl/PROCFS_SYSFS/gcdloader_impl.cpp
)

add_executable(Sysfs${PROJECT_NAME}
    gcdmain.cpp
    Common/gcdapp.cpp
    Common/gcdcore.cpp
    Common/gcdloader.cpp
    Common/gcdparser.cpp
    Impl/PROCFS_SYSFS/gcdcore_impl.cpp
    Impl/PROCFS_SYSFS/gcdloader_impl.cpp
)

target_link_libraries(Ioctl${PROJECT_NAME} PRIVATE Utilities)
target_link_libraries(Procfs${PROJECT_NAME} PRIVATE Utilities)
target_link_libraries(Sysfs${PROJECT_NAME} PRIVATE Utilities)

target_compile_definitions(Ioctl${PROJECT_NAME} PRIVATE IOCTL_GCD)
target_compile_definitions(Procfs${PROJECT_NAME} PRIVATE PROCFS_GCD)
target_compile_definitions(Sysfs${PROJECT_NAME} PRIVATE SYSFS_GCD)

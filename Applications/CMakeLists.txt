# can only compile on Linux
if (NOT WIN32 AND NOT APPLE)
    add_subdirectory(CapsLockLED)
endif()

add_subdirectory(GreatestCommonDivisor)

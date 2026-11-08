project(StopWatch LANGUAGES CXX)

include_directories(
    ../../Utilities
)

add_executable(${PROJECT_NAME}
    stopwatchmain.cpp
    stopwatchcore.cpp
    stopwatchloader.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE Utilities)

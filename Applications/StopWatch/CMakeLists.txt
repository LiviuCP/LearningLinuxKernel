project(StopWatch LANGUAGES CXX)

add_executable(${PROJECT_NAME} stopwatchmain.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE Utilities)

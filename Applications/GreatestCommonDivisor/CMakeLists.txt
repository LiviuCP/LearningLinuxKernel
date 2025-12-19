cmake_minimum_required(VERSION 3.5)

project(GreatestCommonDivisor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(GCD)

add_executable(${PROJECT_NAME}
    gcdmain.cpp
    GCD/gcdcore.cpp
    GCD/gcdloader.cpp
    GCD/gcdparser.cpp
    GCD/gcdutils.cpp
)

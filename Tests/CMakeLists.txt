cmake_minimum_required(VERSION 3.5)

project(Tests LANGUAGES CXX)

find_package(QT NAMES Qt5 Qt6 COMPONENTS Test REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

enable_testing()

add_executable(MappingModuleTests tst_mappingmoduletests.cpp)
add_test(NAME MappingModuleTests COMMAND MappingModuleTests)

target_link_libraries(MappingModuleTests PRIVATE Qt${QT_VERSION_MAJOR}::Test)

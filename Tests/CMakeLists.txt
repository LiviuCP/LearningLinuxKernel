project(Tests LANGUAGES CXX)

find_package(QT NAMES Qt5 Qt6 COMPONENTS Test REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test REQUIRED)

include_directories(../Utilities)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

enable_testing()

add_executable(MappingModuleTests tst_mappingmoduletests.cpp testutils.cpp)
add_test(NAME MappingModuleTests COMMAND MappingModuleTests)

add_executable(StringOpsModuleTests tst_stringopsmoduletests.cpp testutils.cpp)
add_test(NAME StringOpsModuleTests COMMAND StringOpsModuleTests)

add_executable(IoctlStringOpsModuleTests tst_ioctlstringopsmoduletests.cpp testutils.cpp)
add_test(NAME IoctlStringOpsModuleTests COMMAND IoctlStringOpsModuleTests)

target_link_libraries(MappingModuleTests PRIVATE Qt${QT_VERSION_MAJOR}::Test)
target_link_libraries(MappingModuleTests PRIVATE Utilities)

target_link_libraries(StringOpsModuleTests PRIVATE Qt${QT_VERSION_MAJOR}::Test)
target_link_libraries(StringOpsModuleTests PRIVATE Utilities)

target_link_libraries(IoctlStringOpsModuleTests PRIVATE Qt${QT_VERSION_MAJOR}::Test)
target_link_libraries(IoctlStringOpsModuleTests PRIVATE Utilities)

set(RUN_ALL_TESTS RunAllTests)

add_custom_target(${RUN_ALL_TESTS})
target_sources(${RUN_ALL_TESTS} PRIVATE runalltests.sh)

add_custom_command(TARGET ${RUN_ALL_TESTS} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/runalltests.sh ${CMAKE_CURRENT_BINARY_DIR}/runalltests.sh)

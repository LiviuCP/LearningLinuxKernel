project(Utilities LANGUAGES CXX)

add_library(${PROJECT_NAME} ${LIB_TYPE}
    utilities.cpp
    utils.cpp
)

target_compile_definitions(${PROJECT_NAME} PRIVATE UTITILIES_LIBRARY)

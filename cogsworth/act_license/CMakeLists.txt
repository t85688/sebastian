project(License
    LANGUAGES CXX)

add_library(${PROJECT_NAME}
    STATIC
        act_license.hpp)
target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        common::lib
        Qt${QT_VERSION_MAJOR}::Core)
target_set_warnings(
    TARGET ${PROJECT_NAME}
    ENABLE ${ENABLE_WARNINGS}
    AS_ERRORS ${ENABLE_WARNINGS_AS_ERRORS})

add_library(license::lib
    ALIAS ${PROJECT_NAME})

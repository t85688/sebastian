
project(BroadcastSearch LANGUAGES CXX)

# For QT
find_package(
    QT
    NAMES
    Qt6
    Qt5
    COMPONENTS Core Network
    REQUIRED)
find_package(
    Qt${QT_VERSION_MAJOR}
    COMPONENTS Core Network
    REQUIRED)

# Declare project's execute cpp
add_library(${PROJECT_NAME} include/act_broadcast_search.hpp
    src/act_broadcast_search.cpp)

# Declare library alias
add_library(broadcast_search::lib ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/)

target_link_libraries(
    ${PROJECT_NAME}
    common::lib
    southbound::lib
    algorithm::lib
    auto_scan::lib
    core::lib
    Qt${QT_VERSION_MAJOR}::Core)

target_set_warnings(
    TARGET
    ${PROJECT_NAME}
    ENABLE
    ${ENABLE_WARNINGS}
    AS_ERRORS
    ${ENABLE_WARNINGS_AS_ERRORS})

if(BUILD_TEST)
    add_subdirectory(test)
endif()

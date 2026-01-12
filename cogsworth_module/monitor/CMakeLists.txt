project(Monitor
    LANGUAGES CXX)

# Declare project's execute cpp
add_library(${PROJECT_NAME}
    include/act_monitor.hpp
    src/act_monitor.cpp)

# Declare library alias
add_library(monitor::lib
    ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME}
    common::lib
    southbound::lib
    auto_scan::lib
    core::lib
    Qt${QT_VERSION_MAJOR}::Core)

target_set_warnings(
    TARGET ${PROJECT_NAME}
    ENABLE ${ENABLE_WARNINGS}
    AS_ERRORS ${ENABLE_WARNINGS_AS_ERRORS})

if(BUILD_TEST)
    add_subdirectory(test)
endif()
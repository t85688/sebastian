project(TopologyMapping
    LANGUAGES CXX)

# Declare project's execute cpp
add_library(${PROJECT_NAME}
    include/act_topology_mapping.hpp
    src/act_topology_mapping.cpp
    src/act_scan_mapping.cpp)

# Declare library alias
add_library(topology_mapping::lib
    ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/
        ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME}
    common::lib
    southbound::lib
    auto_scan::lib
    sync::lib
    core::lib
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network)

target_set_warnings(
    TARGET ${PROJECT_NAME}
    ENABLE ${ENABLE_WARNINGS}
    AS_ERRORS ${ENABLE_WARNINGS_AS_ERRORS})

if(BUILD_TEST)
endif()

project(AutoProbe
    LANGUAGES CXX)

# Declare project's execute cpp
add_library(${PROJECT_NAME}
    include/act_auto_probe.hpp
    src/act_auto_probe.cpp
    src/act_probe_feature.cpp)

# Declare library alias
add_library(auto_probe::lib ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME}
    common::lib
    southbound::lib
    Qt${QT_VERSION_MAJOR}::Core)

target_set_warnings(
    TARGET ${PROJECT_NAME}
    ENABLE ${ENABLE_WARNINGS}
    AS_ERRORS ${ENABLE_WARNINGS_AS_ERRORS})

if(BUILD_TEST)
    add_subdirectory(test)
endif()


project(Sync LANGUAGES CXX)

# Declare project's execute cpp
add_library(${PROJECT_NAME} include/act_sync.hpp src/act_sync.cpp)

# Declare library alias
add_library(sync::lib ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/
    ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(
    ${PROJECT_NAME}
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
    common::lib
    southbound::lib)

target_set_warnings(
    TARGET
    ${PROJECT_NAME}
    ENABLE
    ${ENABLE_WARNINGS}
    AS_ERRORS
    ${ENABLE_WARNINGS_AS_ERRORS})

if(BUILD_TEST)
    # add_subdirectory(test)
endif()

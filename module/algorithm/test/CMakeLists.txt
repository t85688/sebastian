project(ALGORITHM_UNIT_TEST LANGUAGES CXX)

# enable CTest testing
enable_testing()
include(GoogleTest)

# FOR QT
find_package(
    QT
    NAMES
    Qt6
    Qt5
    COMPONENTS Core
    REQUIRED)
find_package(
    Qt${QT_VERSION_MAJOR}
    COMPONENTS Core
    REQUIRED)

add_executable(${PROJECT_NAME} act_algorithm_test.cpp)

target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC googletest::lib
           algorithm::lib
           Qt${QT_VERSION_MAJOR}::Core
           common::lib)

# Copy ACT algorithm test projects folder to target folder
add_custom_target(
    copy-act-algorithm-test-projects ALL
    COMMAND ${CMAKE_COMMAND} -E remove_directory
            ${EXECUTABLE_OUTPUT_PATH}/act_algorithm_test_projects/
    COMMAND
        ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/act_algorithm_test_projects/
        ${EXECUTABLE_OUTPUT_PATH}/act_algorithm_test_projects)

gtest_discover_tests(${PROJECT_NAME})

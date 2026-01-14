project(Common_UnitTest
    LANGUAGES CXX)
message("Build common testing code")

# enable CTest testing
enable_testing()
include(GoogleTest)
# find_package(GTest)

add_executable(${PROJECT_NAME}
    json_unit_test.cpp
    act_system_test.cpp
    act_stream_test.cpp)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${PROJECT_SOURCE_DIR})

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
        googletest::lib
        googletest_mock::lib
        common::lib
        Qt${QT_VERSION_MAJOR}::Core)

# gtest_add_tests(${PROJECT_NAME} "" AUTO)
gtest_discover_tests(${PROJECT_NAME})

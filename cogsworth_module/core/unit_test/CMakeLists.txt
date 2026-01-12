project(Core_UnitTest
    LANGUAGES CXX)
message("Build core testing code")

# enable CTest testing
enable_testing()
include(GoogleTest)
# find_package(GTest)

add_executable(${PROJECT_NAME}
    act_core_stream_test.cpp
    act_core_test.cpp)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${PROJECT_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        googletest::lib
        core::lib
        Qt${QT_VERSION_MAJOR}::Core)

# gtest_add_tests(${PROJECT_NAME} "" AUTO)
gtest_discover_tests(${PROJECT_NAME})

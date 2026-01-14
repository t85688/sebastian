project(AutoProbe_UnitTest
    LANGUAGES CXX)

# enable CTest testing
enable_testing()
include(GoogleTest)

add_executable(${PROJECT_NAME}
    act_auto_probe_test.cpp)

target_link_libraries(
    ${PROJECT_NAME}
    googletest::lib
    common::lib
    auto_probe::lib)

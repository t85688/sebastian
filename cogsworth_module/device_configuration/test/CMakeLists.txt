project(DEVICE_CONFIGURATION_UNIT_TEST
    LANGUAGES CXX)

# enable CTest testing
enable_testing()
include(GoogleTest)

add_executable(${PROJECT_NAME}
    act_device_configuration_test.cpp)

target_link_libraries(${PROJECT_NAME}
    device_configuration::lib
    googletest::lib
    common::lib)

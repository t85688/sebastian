project(Monitor_UnitTest
    LANGUAGES CXX)

# enable CTest testing
enable_testing()

# include(GoogleTest)

add_executable(${PROJECT_NAME}
    act_monitor_test.cpp)

target_link_libraries(${PROJECT_NAME}
    common::lib
    monitor::lib)

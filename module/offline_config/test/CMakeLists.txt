project(OFFLINE_CONFIG_TEST
    LANGUAGES CXX)

add_executable(${PROJECT_NAME}
    act_offline_config_test.cpp)

target_link_libraries(${PROJECT_NAME}
    common::lib
    offline_config::lib
    Qt${QT_VERSION_MAJOR}::Core)

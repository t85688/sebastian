project(MQTT_CLIENT_TEST
    LANGUAGES CXX)

add_executable(${PROJECT_NAME}
    act_mqtt_client_test.cpp)

target_link_libraries(${PROJECT_NAME}
    common::lib
    mqtt_client::lib
    Qt${QT_VERSION_MAJOR}::Core)

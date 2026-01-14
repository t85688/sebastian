project(MQTTClient
  LANGUAGES CXX)

# Declare project's execute cpp
add_library(${PROJECT_NAME}
  include/act_mqtt_client.hpp
  src/act_mqtt_client.cpp)

# Declare library alias
add_library(mqtt_client::lib
  ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${PROJECT_SOURCE_DIR}/
    ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    common::lib
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Mqtt)

target_set_warnings(
  TARGET ${PROJECT_NAME}
  ENABLE ${ENABLE_WARNINGS}
  AS_ERRORS ${ENABLE_WARNINGS_AS_ERRORS})

add_subdirectory(test)

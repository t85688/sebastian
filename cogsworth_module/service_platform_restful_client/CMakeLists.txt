project(ServicePlatform_Client
  LANGUAGES CXX)

add_library(
  ${PROJECT_NAME}
  include/act_service_platform_client.hpp
  include/act_service_platform_client_handler.hpp
  include/act_service_platform_client_agent.hpp
  src/act_service_platform_client.cpp)

add_library(service_platform_client::lib
  ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    oatpp::oatpp-curl
    oatpp::oatpp
  PRIVATE
    common::lib
    core::lib
    Qt${QT_VERSION_MAJOR}::Core)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${PROJECT_SOURCE_DIR}/include)

target_set_warnings(
  TARGET ${PROJECT_NAME}
  ENABLE ${ENABLE_WARNINGS}
  AS_ERRORS ${ENABLE_WARNINGS_AS_ERRORS})

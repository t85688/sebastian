project(MAF_Client
  LANGUAGES CXX)

# Declare project's execute cpp
add_library(
  ${PROJECT_NAME}
  include/act_maf_client.hpp
  include/act_maf_client_agent.hpp
  include/act_maf_client_handler.hpp
  src/act_maf_client.cpp)

# Declare library alias
add_library(maf_client::lib
  ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    oatpp::oatpp-curl
    oatpp::oatpp
  PRIVATE
    common::lib
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${PROJECT_SOURCE_DIR}/include)

target_set_warnings(
  TARGET ${PROJECT_NAME}
  ENABLE ${ENABLE_WARNINGS}
  AS_ERRORS ${ENABLE_WARNINGS_AS_ERRORS})

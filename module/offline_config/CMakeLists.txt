project(OfflineConfig
  LANGUAGES CXX)

# Declare project's execute cpp
add_library(${PROJECT_NAME}
  include/act_offline_config.hpp
  src/act_offline_config.cpp
  src/act_offline_config_feature.cpp
)

# Declare library alias
add_library(offline_config::lib
  ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME}
  common::lib
  maf_client::lib
  restful_client_handler::lib
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::Network)

target_set_warnings(
  TARGET ${PROJECT_NAME}
  ENABLE ${ENABLE_WARNINGS}
  AS_ERRORS ${ENABLE_WARNINGS_AS_ERRORS})

if(BUILD_TEST)
  add_subdirectory(test)
endif()

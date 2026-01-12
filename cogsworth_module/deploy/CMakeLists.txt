project(Deploy
    LANGUAGES CXX)

# Declare project's execute cpp
add_library(${PROJECT_NAME}
    include/act_deploy.hpp
    include/act_deploy_path_data.hpp
    src/act_deploy.cpp
    src/act_deploy_path_data.cpp
    src/act_deploy_ini.cpp
    src/act_deploy_configure.cpp
    ${PROJECT_SOURCE_DIR}/include/act_deploy_result.hpp)

# Declare library alias
add_library(deploy::lib
    ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME}
    common::lib
    southbound::lib
    compare::lib
    algorithm::lib
    Qt${QT_VERSION_MAJOR}::Core)

target_set_warnings(
    TARGET ${PROJECT_NAME}
    ENABLE ${ENABLE_WARNINGS}
    AS_ERRORS ${ENABLE_WARNINGS_AS_ERRORS})

if(BUILD_TEST)
    add_subdirectory(test)
endif()

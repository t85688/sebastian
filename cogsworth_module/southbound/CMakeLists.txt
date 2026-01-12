project(Southbound
    LANGUAGES CXX)

add_subdirectory(restful_client)
add_subdirectory(new_moxa_command)
add_subdirectory(snmp)

# # Declare project's execute cpp
add_library(
    ${PROJECT_NAME} STATIC
    include/act_southbound.hpp
    include/act_southbound_result.hpp
    src/act_southbound_error_handler.cpp
    src/act_southbound_action.cpp
    src/act_southbound_feature.cpp
    src/act_southbound_probe.cpp
    src/act_southbound_scan.cpp
    src/act_southbound_config.cpp
    src/act_southbound.cpp
)

# Declare library alias
add_library(southbound::lib
    ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/
        ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME}
    common::lib
    core::lib
    snmp_handler::lib
    restful_client_handler::lib
    new_moxa_command_handler::lib
    Qt${QT_VERSION_MAJOR}::Core)

target_set_warnings(
    TARGET ${PROJECT_NAME}
    ENABLE ${ENABLE_WARNINGS}
    AS_ERRORS ${ENABLE_WARNINGS_AS_ERRORS})

if(BUILD_TEST)
    add_subdirectory(test)
endif()

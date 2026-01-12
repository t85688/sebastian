project(Common
    LANGUAGES CXX)

file(GLOB DEPLOY_ENTRY_FOLDER "deploy_entry/*.hpp")
file(GLOB DEVICE_CONFIGURATION_FOLDER "device_configuration/*.hpp")
file(GLOB GCL_FOLDER "gcl/*.hpp")
file(GLOB ROUTING_FOLDER "routing/*.hpp")
file(
    GLOB
    STREAM_FOLDER
    "stream/*.hpp"
    "stream/*/*.hpp")
file(GLOB TOPOLOGY_FOLDER "topology/*.hpp")
file(GLOB AUTO_PROBE_FOLDER "auto_probe/*.hpp")
file(GLOB PROFILES_FOLDER "profiles/*.hpp")
file(GLOB DEVICE_MODULES_FOLDER "device_modules/*.hpp")

add_library(
    ${PROJECT_NAME} STATIC
    ${QSERIALIZER_HEADER}
    json/act_json.hpp
    json/json_utils.hpp
    simplecrypt/simplecrypt.cpp
    simplecrypt/simplecrypt.h
    act_project.cpp
    act_project.hpp
    act_status.hpp
    act_service_platform_request.hpp
    act_system.hpp
    act_class_based.hpp
    act_user.hpp
    act_group.hpp
    act_feature.hpp
    act_firmware.hpp
    act_host_adapter.hpp
    act_import_export_project.hpp
    logger/act_logutils.cpp
    logger/act_logutils.h
    act_algorithm_configuration.hpp
    act_monitor_data.hpp
    act_network_baseline.hpp
    act_common_feature.hpp
    act_intelligent_request.hpp
    act_device_event_log.hpp
    act_event_log.hpp
    act_temperature.hpp

    act_utilities.hpp
    act_computed_result.hpp
    act_ws_cmd.hpp
    act_ws_client.hpp
    act_notification_msg.hpp
    act_scan_ip_range.hpp
    act_broadcast_search_config.hpp
    act_vlan_view.hpp
    act_vlan_config.hpp
    act_deploy_parameter.hpp
    act_device_connection.hpp
    act_snmp_trap_configuration.hpp
    act_snmp_trap_message.hpp

    act_job.hpp
    ${DEPLOY_ENTRY_FOLDER}
    ${DEVICE_CONFIGURATION_FOLDER}
    ${GCL_FOLDER}
    ${ROUTING_FOLDER}
    ${STREAM_FOLDER}
    ${TOPOLOGY_FOLDER}
    ${AUTO_PROBE_FOLDER}
    ${PROFILES_FOLDER}
    ${DEVICE_MODULES_FOLDER})

# option(JWT_DISABLE_PICOJSON "Do not provide the picojson template specialize" ON)
# if(JWT_DISABLE_PICOJSON)
# target_compile_definitions(${PROJECT_NAME} INTERFACE JWT_DISABLE_PICOJSON)
# endif()
add_library(common::lib ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        jwt-cpp::jwt-cpp
        OpenSSL::Crypto
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::WebSockets)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/
        ${PROJECT_SOURCE_DIR}/jwt-cpp
        ${PROJECT_SOURCE_DIR}/json
        ${PROJECT_SOURCE_DIR}/simplecrypt
        ${PROJECT_SOURCE_DIR}/logger
        ${PROJECT_SOURCE_DIR}/device_modules
        ${PROJECT_SOURCE_DIR}/deploy_entry
        ${PROJECT_SOURCE_DIR}/device_configuration
        ${PROJECT_SOURCE_DIR}/gcl
        ${PROJECT_SOURCE_DIR}/routing
        ${PROJECT_SOURCE_DIR}/stream
        ${PROJECT_SOURCE_DIR}/topology
        ${PROJECT_SOURCE_DIR}/auto_probe
        ${PROJECT_SOURCE_DIR}/profiles
        ${QSERIALIZER_INCLUDE_DIRS}
        ${QUAZIP_INCLUDE_DIRS})

target_set_warnings(
    TARGET ${PROJECT_NAME}
    ENABLE ${ENABLE_WARNINGS}
    AS_ERRORS ${ENABLE_WARNINGS_AS_ERRORS})

if(BUILD_TEST)
    add_subdirectory(unit_test)
endif()

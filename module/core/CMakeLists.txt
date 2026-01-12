project(Core
    LANGUAGES CXX)

add_library(${PROJECT_NAME} STATIC
    include/act_core.hpp
    src/act_core.cpp
    src/act_core_quazip.cpp
    src/act_core_system.cpp
    src/act_core_user.cpp
    src/act_core_login.cpp
    src/act_core_project.cpp
    src/act_core_management_interface.cpp
    src/act_core_project_setting.cpp
    src/act_core_connection_config.cpp
    src/act_core_device.cpp
    src/act_core_event_log.cpp
    src/act_core_device_setting.cpp
    src/act_core_device_module.cpp
    src/act_core_device_profile.cpp
    src/act_core_default_device_profile.cpp
    src/act_core_firmware.cpp
    src/act_core_host.cpp
    src/act_core_link.cpp
    src/act_core_group.cpp
    src/act_core_stream.cpp
    src/act_core_class_based.cpp
    src/act_core_compute.cpp
    src/act_core_compare.cpp
    src/act_core_deploy.cpp
    src/act_core_manufacture_deploy.cpp
    src/act_core_network_baseline.cpp
    src/act_core_sync_device.cpp
    src/act_core_module.cpp
    src/act_core_monitor.cpp
    src/act_core_monitor_process_engine.cpp
    src/act_core_scan_topology.cpp
    src/act_core_broadcast_search.cpp
    src/act_core_topology_mapping.cpp
    src/act_core_scan_mapping.cpp
    src/act_core_redundant_group.cpp
    src/act_core_redundant_swift.cpp
    src/act_core_device_configuration.cpp
    src/act_core_service_platform.cpp
    src/act_core_import_export_device_config.cpp
    src/act_core_intelligent_device_configuration.cpp
    src/act_core_ws_test.cpp
    src/act_core_chamberlain.cpp
    src/act_core_feature_profile.cpp
    src/act_core_service_profile.cpp
    src/act_core_firmware_feature_profile.cpp
    src/act_core_auto_probe.cpp
    src/act_core_device_icon.cpp
    src/act_core_static_forward_config.cpp
    src/act_core_vlan_config.cpp
    src/act_core_vlan_view.cpp
    src/act_core_device_config.cpp
    src/act_core_intelligent.cpp
    src/act_core_command_line_interface.cpp
    src/act_core_topology.cpp
    src/act_core_traffic.cpp
    src/act_core_notification_msg_listener.cpp
    src/act_core_notification_tmp.cpp
    src/act_core_undo_redo.cpp
    src/act_core_worker.cpp
    src/act_core_snmp_resource.cpp
    src/act_core_device_ini_config.cpp
)

add_library(core::lib ALIAS ${PROJECT_NAME})

list(
    APPEND
    ADDITIONAL_LIBS
    oatpp::oatpp-websocket
    http_server::lib
)

list(
    APPEND
    ADDITIONAL_LIBS
    opcua_class_based::lib)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        license::lib
        ${ADDITIONAL_LIBS}
        QuaZip::QuaZip
        auto_scan::lib
        algorithm::lib
        deploy::lib
        compare::lib
        monitor::lib
        sync::lib
        broadcast_search::lib
        topology_mapping::lib
        device_configuration::lib
        offline_config::lib
        auto_probe::lib
        database::lib
        intelligent::lib
        maf_client::lib
        service_platform_client::lib
        southbound::lib
        common::lib
        mqtt_client::lib
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Concurrent
        Qt${QT_VERSION_MAJOR}::Network)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include)

target_set_warnings(
    TARGET ${PROJECT_NAME}
    ENABLE ${ENABLE_WARNINGS}
    AS_ERRORS ${ENABLE_WARNINGS_AS_ERRORS})

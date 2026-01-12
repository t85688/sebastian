project(HttpServer
    LANGUAGES CXX)

add_definitions(

    # define path to swagger-ui static resources folder
    # -DOATPP_SWAGGER_RES_PATH="${oatpp-swagger_INCLUDE_DIRS}/../bin/oatpp-swagger/res"
    -DOATPP_SWAGGER_RES_PATH="./swagger")

# add library
add_library(${PROJECT_NAME} STATIC
    src/App.cpp
    src/http_utils.cpp
    src/http_utils.h
    src/websocket/act_ws_listener.cpp
    src/websocket/act_ws_listener.hpp
    src/app_component.hpp
    src/swagger_component.hpp
    src/controller/act_class_based_controller.hpp

    src/controller/act_ws_controller.hpp
    src/controller/act_compute_controller.hpp
    src/controller/act_controller.cpp
    src/controller/act_controller.hpp
    src/controller/act_monitor_controller.hpp
    src/controller/act_device_controller.hpp
    src/controller/act_device_profile_controller.hpp
    src/controller/act_device_setting_controller.hpp
    src/controller/act_deploy_controller.hpp
    src/controller/act_manufacture_controller.hpp
    src/controller/act_network_baseline_controller.hpp
    src/controller/act_device_module_controller.hpp
    src/controller/act_event_log_controller.hpp
    src/controller/act_firmware_controller.hpp
    src/controller/act_service_profile_controller.hpp
    src/controller/act_service_platform_controller.hpp
    src/controller/act_license_controller.hpp
    src/controller/act_link_controller.hpp
    src/controller/act_login_controller.hpp
    src/controller/act_project_controller.hpp
    src/controller/act_management_interface_controller.hpp
    src/controller/act_project_setting_controller.hpp
    src/controller/act_stream_controller.hpp
    src/controller/act_system_controller.hpp
    src/controller/act_swagger_controller.hpp
    src/controller/act_user_controller.hpp
    src/controller/act_static_forward_config_controller.hpp
    src/controller/act_vlan_config_controller.hpp
    src/controller/act_vlan_view_controller.hpp
    src/controller/act_intelligent_controller.hpp
    src/controller/act_topology_controller.hpp
    src/controller/act_redundant_swift_controller.hpp
    src/controller/act_traffic_controller.hpp
    src/controller/act_host_controller.hpp
    src/dto/act_class_based_dto.hpp
    src/dto/act_compute_dto.hpp
    src/dto/act_coordinate_dto.hpp
    src/dto/act_device_dto.hpp
    src/dto/act_device_setting_dto.hpp
    src/dto/act_device_profile_dto.hpp
    src/dto/act_firmware_feature_profile_dto.hpp
    src/dto/act_service_profile_dto.hpp
    src/dto/act_service_platform_dto.hpp
    src/dto/act_device_view_result_dto.hpp
    src/dto/act_model_prices_dto.hpp
    src/dto/act_event_log_dto.hpp
    src/dto/act_feature_dto.hpp
    src/dto/act_firmware_dto.hpp
    src/dto/act_gate_control_dto.hpp
    src/dto/act_gcl_result_dto.hpp
    src/dto/act_interface_dto.hpp
    src/dto/act_interval_dto.hpp
    src/dto/act_license_dto.hpp
    src/dto/act_link_dto.hpp
    src/dto/act_user_dto.hpp
    src/dto/act_login_dto.hpp
    src/dto/act_project_dto.hpp
    src/dto/act_temperature_dto.hpp
    src/dto/act_routing_result_dto.hpp
    src/dto/act_snmp_configuration_dto.hpp
    src/dto/act_status_dto.hpp
    src/dto/act_stream_dto.hpp
    src/dto/act_stream_view_result_dto.hpp
    src/dto/act_monitor_dto.hpp
    src/dto/act_network_baseline_dto.hpp
    src/dto/act_sfp_list_dto.hpp
    src/dto/act_user_dto.hpp
    src/dto/act_device_config_dto.hpp
    src/dto/act_vlan_view_dto.hpp
    src/dto/act_vlan_config_dto.hpp
    src/dto/act_intelligent_dto.hpp
    src/dto/act_topology_dto.hpp
    src/dto/act_undo_redo_dto.hpp
    src/dto/act_redundant_swift_dto.hpp
    src/dto/act_traffic_dto.hpp
    src/dto/act_ethernet_module_dto.hpp
    src/dto/act_sfp_module_dto.hpp
    src/dto/act_power_module_dto.hpp
    src/controller/StaticController.hpp
    src/controller/UserController.hpp
    src/dto/act_power_device_profile_dto.hpp
    src/dto/act_software_license_profile_dto.hpp

    # Test components
    # src/controller/act_test_controller.hpp

    # Test components
    # src/AppComponent.hpp
    # src/controller/MyController.cpp
    # src/controller/MyController.hpp
    # src/dto/DTOs.hpp
)

# add library alias
add_library(http_server::lib
    ALIAS ${PROJECT_NAME})

target_compile_options(${PROJECT_NAME}
    PRIVATE
        /bigobj)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        oatpp::oatpp-websocket
        oatpp::oatpp-openssl
        oatpp::oatpp-swagger
        oatpp::oatpp
        common::lib
        license::lib
        core::lib
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR})

if(BUILD_TEST)
    add_executable(${PROJECT_NAME}-test
        main.cpp)
    target_link_libraries(${PROJECT_NAME}-test
        PUBLIC
            oatpp::oatpp-websocket
            oatpp::oatpp-openssl
            oatpp::oatpp-swagger
            oatpp::oatpp)
endif()

target_set_warnings(
    TARGET ${PROJECT_NAME}
    ENABLE ${ENABLE_WARNINGS}
    AS_ERRORS ${ENABLE_WARNINGS_AS_ERRORS})

set(PROJECT_RES_FLAVORS
    Chamberlain
    MoxaACT)

foreach(RES_FLAVOR IN LISTS PROJECT_RES_FLAVORS)
    set(FLAVOR_TARGET "Cogsworth_${RES_FLAVOR}")

    set(PROJECT_RESOURCE_SWAGGER_DIR ${OATPP_SWAGGER_RESOURCE_DIR})
    add_custom_target(
        "${FLAVOR_TARGET}_copy-swagger-res" ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_RESOURCE_SWAGGER_DIR}
        $<TARGET_FILE_DIR:${FLAVOR_TARGET}>/swagger)
    install(DIRECTORY ${PROJECT_RESOURCE_SWAGGER_DIR}
        DESTINATION bin/swagger)
endforeach()

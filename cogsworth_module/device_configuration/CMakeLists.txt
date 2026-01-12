project(DeviceConfiguration
    LANGUAGES CXX)

# Declare project's execute cpp
add_library(${PROJECT_NAME}
    include/act_device_configuration.hpp
    src/act_device_configuration.cpp
    src/act_device_config_command_line.cpp
    src/act_device_config_commission.cpp
    src/act_device_config_ip.cpp
    src/act_device_config_reboot.cpp
    src/act_device_config_factory_default.cpp
    src/act_device_config_firmware_upgrade.cpp
    src/act_device_config_enable_snmp.cpp
    src/act_device_config_import.cpp
    src/act_device_config_export.cpp
    src/act_device_config_locator.cpp
    src/act_device_config_event_log.cpp)

# Declare library alias
add_library(device_configuration::lib
    ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME}
    core::lib
    common::lib
    southbound::lib
    Qt${QT_VERSION_MAJOR}::Core)

target_set_warnings(
    TARGET ${PROJECT_NAME}
    ENABLE ${ENABLE_WARNINGS}
    AS_ERRORS ${ENABLE_WARNINGS_AS_ERRORS})

if(BUILD_TEST)
    # add_subdirectory(test)
endif()

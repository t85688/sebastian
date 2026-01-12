project(SNMP_UNIT_TEST
    LANGUAGES CXX)

add_executable(${PROJECT_NAME}
    act_snmp_test.cpp)

target_link_libraries(${PROJECT_NAME}
    common::lib
    snmp_handler::lib
    Qt${QT_VERSION_MAJOR}::Core)

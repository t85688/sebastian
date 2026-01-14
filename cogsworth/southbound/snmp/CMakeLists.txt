project(SNMP
    LANGUAGES CXX)

# Declare project's execute cpp
add_library(
    ${PROJECT_NAME}
    include/act_snmp_result.hpp
    include/act_snmp_set_entry.hpp
    src/act_snmp_handler.cpp

    src/agents/act_snmp_agent.cpp
    src/agents/act_snmpwalk.cpp
    src/agents/act_snmpset.cpp
)

# Declare library alias
add_library(snmp_handler::lib ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        NetSNMP::NetSNMP
    PRIVATE
        common::lib
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Network)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/include/agents)

target_set_warnings(
    TARGET ${PROJECT_NAME}
    ENABLE ${ENABLE_WARNINGS}
    AS_ERRORS ${ENABLE_WARNINGS_AS_ERRORS})

if(BUILD_TEST)
    add_subdirectory(test)
endif()

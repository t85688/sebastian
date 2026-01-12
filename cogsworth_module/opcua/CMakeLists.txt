project(OpcuaServer
    LANGUAGES CXX)

file(GLOB OPCUA_SOURCE
    ${PROJECT_SOURCE_DIR}/*.cpp
    ${PROJECT_SOURCE_DIR}/*.h)

# Add a library with the above sources
add_library(${PROJECT_NAME} ${LINK_TYPE} ${OPCUA_SOURCE})

add_library(opcua_server::lib ALIAS ${PROJECT_NAME})

# !!!!!Make header file can get the header files of other project
target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR})

# 連結其他project的library及自己的.h路徑
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        UASDKCPP::uamodule
        UASDKCPP::coremodule
        UASDKCPP::uapkicpp
        UASDKCPP::uabasecpp
        UASDKCPP::uastack
        UASDKCPP::xmlparsercpp
        Qt${QT_VERSION_MAJOR}::Core)

target_set_warnings(
    TARGET ${PROJECT_NAME}
    ENABLE ${ENABLE_WARNINGS}
    AS_ERRORS ${ENABLE_WARNINGS_AS_ERRORS})

add_subdirectory(opcua_class_based)

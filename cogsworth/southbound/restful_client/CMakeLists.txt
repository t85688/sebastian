project(RESTfulClient
    LANGUAGES CXX)

# Declare project's execute cpp
add_library(${PROJECT_NAME}
    STATIC
        include/client/act_moxa_iei_client.hpp
        include/agents/act_moxa_iei_client_agent.hpp
        src/act_restful_client_handler.cpp)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        oatpp::oatpp-curl
        oatpp::oatpp-openssl
        oatpp::oatpp
    PRIVATE
        common::lib
        core::lib
        Qt${QT_VERSION_MAJOR}::Core)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include)

target_set_warnings(
    TARGET ${PROJECT_NAME}
    ENABLE ${ENABLE_WARNINGS}
    AS_ERRORS ${ENABLE_WARNINGS_AS_ERRORS})

# Declare library alias
add_library(restful_client_handler::lib
    ALIAS ${PROJECT_NAME})

if(BUILD_TEST)
    add_subdirectory(test)
endif()

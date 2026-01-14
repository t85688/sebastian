project(Intelligent
    LANGUAGES CXX)

# Declare project's execute cpp
add_library(${PROJECT_NAME}
    include/act_moxa_ai_client.hpp
    include/act_moxa_ai_client_agent.hpp
    include/act_intelligent.hpp
    src/act_intelligent.cpp)

# Declare library alias
add_library(intelligent::lib
    ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        oatpp::oatpp-curl
        oatpp::oatpp
    PRIVATE
        common::lib
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Network)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include)

target_set_warnings(
    TARGET ${PROJECT_NAME}
    ENABLE ${ENABLE_WARNINGS}
    AS_ERRORS ${ENABLE_WARNINGS_AS_ERRORS})

if(BUILD_TEST)
    # add_subdirectory(test)
endif()

project(OpcuaClassBased LANGUAGES CXX)

file(
    GLOB
    OPCUA_SOURCE
    ${PROJECT_SOURCE_DIR}/*.cpp
    ${PROJECT_SOURCE_DIR}/*.h
    ${PROJECT_SOURCE_DIR}/generated_code/*.cpp
    ${PROJECT_SOURCE_DIR}/generated_code/*.h
    ${PROJECT_SOURCE_DIR}/moxa/*.cpp
    ${PROJECT_SOURCE_DIR}/moxa/*.h)

# find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
# find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)

# Add a library with the above sources
add_library(${PROJECT_NAME} ${LINK_TYPE} ${OPCUA_SOURCE})
add_library(opcua_class_based::lib ALIAS ${PROJECT_NAME})

# !!!!!Make header file can get the header files of other project
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}
    PRIVATE ${PROJECT_SOURCE_DIR}/generated_code ${PROJECT_SOURCE_DIR}/moxa)

# #連結其他project的library及自己的.h路徑
target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC opcua_server::lib
           # common::lib
           core::lib
           Qt${QT_VERSION_MAJOR}::WebSockets
           Qt${QT_VERSION_MAJOR}::Core)

target_set_warnings(
    TARGET
    ${PROJECT_NAME}
    ENABLE
    ${ENABLE_WARNINGS}
    AS_ERRORS
    ${ENABLE_WARNINGS_AS_ERRORS})

#if(ENABLE_SANITIZE_ADDR OR #ENABLE_SANITIZE_UNDEF)
#    include(Sanitizer)
#    add_sanitizer_flags()
#endif()

if(${ENABLE_LTO})
    target_enable_lto(${PROJECT_NAME} optimized)
endif()
# install(
#     TARGETS ${PROJECT_NAME}
#     RUNTIME DESTINATION ${EXECUTABLE_OUTPUT_PATH}
#     LIBRARY DESTINATION ${LIBRARY_OUTPUT_PATH})

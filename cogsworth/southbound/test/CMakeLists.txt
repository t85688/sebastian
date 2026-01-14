project(SOUTHBOUND_UNIT_TEST
    LANGUAGES CXX)

# Enable CTest testing
enable_testing()
include(GoogleTest)

add_executable(${PROJECT_NAME}
    act_southbound_test.cpp)

target_link_libraries(${PROJECT_NAME}
    googletest::lib
    common::lib
    southbound::lib
    Qt${QT_VERSION_MAJOR}::Core)

# Copy fake folder to target folder
add_custom_target(
    copy-southbound-fake ALL
    COMMAND ${CMAKE_COMMAND} -E remove_directory
            ${EXECUTABLE_OUTPUT_PATH}/southbound_fake/
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/fake/
            ${EXECUTABLE_OUTPUT_PATH}/southbound_fake)


project(DEPLOY_UNIT_TEST LANGUAGES CXX)

# enable CTest testing
enable_testing()
include(GoogleTest)

# FOR QT
find_package(
    QT
    NAMES
    Qt6
    Qt5
    COMPONENTS Core
    REQUIRED)
find_package(
    Qt${QT_VERSION_MAJOR}
    COMPONENTS Core
    REQUIRED)

add_executable(${PROJECT_NAME} act_deploy_test.cpp)

target_link_libraries(
    ${PROJECT_NAME}
    deploy::lib
    googletest::lib
    common::lib
    Qt${QT_VERSION_MAJOR}::Core)

# Copy fake folder to target folder
add_custom_target(
    copy-deploy-fake ALL
    COMMAND ${CMAKE_COMMAND} -E remove_directory
            ${EXECUTABLE_OUTPUT_PATH}/deploy_fake/
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/fake/
            ${EXECUTABLE_OUTPUT_PATH}/deploy_fake)


project(NEW_MOXA_COMMAND_UNIT_TEST LANGUAGES CXX)

# set(CMAKE_CXX_FLAGS "-lwsock32")

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

add_executable(${PROJECT_NAME} act_new_moxa_command_test.cpp)

# add_definitions(-DCURL_STATICLIB)
target_link_libraries(
    ${PROJECT_NAME}
    new_moxa_command_handler::lib
    googletest::lib
    common::lib
    Qt${QT_VERSION_MAJOR}::Core)


project(AUTO_SCAN_UNIT_TEST LANGUAGES CXX)

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

add_executable(${PROJECT_NAME} act_auto_scan_test.cpp)

target_link_libraries(
    ${PROJECT_NAME}
    googletest::lib
    common::lib
    auto_scan::lib
    Qt${QT_VERSION_MAJOR}::Core)

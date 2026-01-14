project(Database
    LANGUAGES CXX)

add_library(${PROJECT_NAME} STATIC
    include/act_db.hpp
    src/act_db.cpp
    src/act_system_db.cpp
    include/act_system_db.hpp
    src/act_user_db.cpp
    include/act_user_db.hpp
    src/act_project_db.cpp
    include/act_project_db.hpp
    src/act_firmware_db.cpp
    include/act_firmware_db.hpp
    src/act_topology_db.cpp
    include/act_topology_db.hpp
    src/act_network_baseline_db.cpp
    include/act_network_baseline_db.hpp
)

add_library(database::lib
    ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        common::lib
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Network)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/
        ${PROJECT_SOURCE_DIR}/include)

target_set_warnings(
    TARGET ${PROJECT_NAME}
    ENABLE ${ENABLE_WARNINGS}
    AS_ERRORS ${ENABLE_WARNINGS_AS_ERRORS})

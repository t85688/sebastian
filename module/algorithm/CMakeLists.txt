project(Algorithm LANGUAGES CXX)

add_library(
    ${PROJECT_NAME} STATIC
    src/act_algorithm.cpp
    include/act_algorithm.hpp
    include/act_schedule.hpp
    include/act_schedule_config.hpp
    include/act_schedule_stream.hpp
    include/act_schedule_result.hpp
    include/act_schedule_feasibility.hpp
    src/act_schedule.cpp
    src/act_schedule_config.cpp
    src/act_schedule_result.cpp
    src/act_schedule_stream.cpp
    src/act_schedule_feasibility.cpp)

# Declare library alias of this sub-project
add_library(algorithm::lib ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
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
    add_subdirectory(test)

    add_executable(${PROJECT_NAME}_test
        main.cpp)
    target_link_libraries(${PROJECT_NAME}_test
        PUBLIC
            algorithm::lib
            common::lib
            Qt${QT_VERSION_MAJOR}::Core)

    target_include_directories(${PROJECT_NAME}_test
        PUBLIC
            ${PROJECT_SOURCE_DIR}/include)
endif()

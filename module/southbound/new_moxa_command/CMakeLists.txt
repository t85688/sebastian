project(ACTNewMoxaCommand LANGUAGES CXX)

# Declare project's execute cpp
add_library(${PROJECT_NAME} STATIC
    src/act_new_moxa_command_handler.cpp)

# Declare library alias
add_library(new_moxa_command_handler::lib
    ALIAS ${PROJECT_NAME})

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include/
        ${NEWMOXACOMMAND_INCLUDE_DIRS}
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
        ${NEWMOXACOMMAND_LIBRARIES}
        common::lib
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Network)

target_set_warnings(
    TARGET ${PROJECT_NAME}
    ENABLE ${ENABLE_WARNINGS}
    AS_ERRORS ${ENABLE_WARNINGS_AS_ERRORS})

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND
        ${CMAKE_COMMAND} -E make_directory ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_BUILD_TYPE}/
    COMMAND
        ${CMAKE_COMMAND} -E copy_if_different ${NEWMOXACOMMAND_NEWMOXACOMMAND_BIN_PATH}
        ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_BUILD_TYPE}/)
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND
        ${CMAKE_COMMAND} -E make_directory ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_BUILD_TYPE}/dll/
    COMMAND
        ${CMAKE_COMMAND} -E copy_if_different ${NEWMOXACOMMAND_MOXACOMMAND_BIN_PATH}
        ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_BUILD_TYPE}/dll/)
install(
    FILES ${NEWMOXACOMMAND_NEWMOXACOMMAND_BIN_PATH}
    DESTINATION bin)
install(
    FILES ${NEWMOXACOMMAND_MOXACOMMAND_BIN_PATH}
    DESTINATION bin/dll)

if(BUILD_TEST)
    add_subdirectory(test)
endif()

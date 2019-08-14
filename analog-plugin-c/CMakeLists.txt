cmake_minimum_required(VERSION 3.14)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")
project(analog_plugin_c C)

#set(USE_LOCAL_HIDAPI YES)
set(HIDAPI_USE_UDEV_HIDRAW YES)
set(CMAKE_C_STANDARD 99)
set(analog_sdk_path ${CMAKE_CURRENT_SOURCE_DIR}/../analog-sdk)
set(plugn_includes ${analog_sdk_path}/includes)

add_subdirectory(includes)
if(NOT TARGET hidapi)
    message(FATAL_ERROR "This app doesn't do anything without HIDAPI - fix the errors above!")
endif()

execute_process(COMMAND cargo make --cwd ${CMAKE_CURRENT_SOURCE_DIR}/../analog-sdk/wooting-analog-sdk-common)
add_library(analog_plugin_c SHARED src/plugin.c ${plugn_includes}/plugin.h)
target_link_directories(analog_plugin_c PRIVATE ${analog_sdk_path}/target/debug)
target_include_directories(analog_plugin_c PRIVATE ${plugn_includes})
target_link_libraries(analog_plugin_c PRIVATE analog_sdk_common)
target_link_libraries(analog_plugin_c PRIVATE hidapi)


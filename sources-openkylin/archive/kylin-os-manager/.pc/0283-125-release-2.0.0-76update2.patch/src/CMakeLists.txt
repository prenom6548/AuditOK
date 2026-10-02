cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -g")

if (CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    add_definitions(-DDEBUG_MODE)
endif()

execute_process(COMMAND head -n 1 ${TOP_DIR}/debian/changelog OUTPUT_VARIABLE CHANGELOG_HEAD)
string(REGEX MATCH "\\(([^)]+)\\)" VERSION_STRING ${CHANGELOG_HEAD})
string(REPLACE "(" "" VERSION ${VERSION_STRING})
string(REPLACE ")" "" VERSION ${VERSION})
if(NOT DEFINED VERSION OR "${VERSION}" STREQUAL "")
    message(FATAL_ERROR "VERSION is not defined or is empty")
endif()

configure_file(
        "${CMAKE_CURRENT_LIST_DIR}/config.h.in"
        "${CMAKE_CURRENT_LIST_DIR}/config.h"
)

add_subdirectory(application)
#add_subdirectory(process_manager)
#add_subdirectory(tray_service)

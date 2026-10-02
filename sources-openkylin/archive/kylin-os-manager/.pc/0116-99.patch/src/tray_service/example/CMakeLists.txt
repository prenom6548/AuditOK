cmake_minimum_required(VERSION 3.5)

project(kylin-os-manager-tray-example)

set(EXAMPLE_DIR ${CMAKE_CURRENT_LIST_DIR})

#set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTORCC ON)
#set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -g -O0")

include_directories(${EXAMPLE_DIR})
include_directories(${EXAMPLE_DIR}/interface/)
include_directories(/usr/include/kylin-os-manager-plugin/)

set(SRCS
        ${EXAMPLE_DIR}/interface/kom_tray_interface.cpp
        ${EXAMPLE_DIR}/connector.cpp
        ${EXAMPLE_DIR}/protocol.cpp)

add_library(${PROJECT_NAME} SHARED ${SRCS})

install(TARGETS ${PROJECT_NAME} DESTINATION /opt/kylin-os-manager/tray-plugins)

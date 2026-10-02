cmake_minimum_required(VERSION 3.16)

project(kylin-os-manager-application-example)

set(ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/../../)
set(EXAMPLE_DIR ${CMAKE_CURRENT_LIST_DIR})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_FLAGS "-Wall -g -rdynamic")
set(CMAKE_CXX_STANDARD 17)

include_directories(${EXAMPLE_DIR})
include_directories(${ROOT_DIR}/interface/)
include_directories(${ROOT_DIR}/3rdparty/kyplugin/)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(SRCS
	"${EXAMPLE_DIR}/connector.cpp"
	"${EXAMPLE_DIR}/protocol.cpp"
	"${EXAMPLE_DIR}/mainwindow.cpp"
	"${ROOT_DIR}/interface/kom_application_interface.cpp")

add_library(${PROJECT_NAME} SHARED ${SRCS})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets)

install(TARGETS ${PROJECT_NAME} DESTINATION /opt/kylin-os-manager/plugins/)

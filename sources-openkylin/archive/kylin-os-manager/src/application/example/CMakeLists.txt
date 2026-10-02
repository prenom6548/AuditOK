cmake_minimum_required(VERSION 3.5)

project(kylin-os-manager-application-example)

set(EXAMPLE_DIR ${CMAKE_CURRENT_LIST_DIR})
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC  ON)
set(CMAKE_AUTOUIC  ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -g -O0")

include_directories(${EXAMPLE_DIR})
include_directories(${EXAMPLE_DIR}/interface/)
include_directories(/usr/include/kylin-os-manager-plugin/)

find_package(PkgConfig)
find_package(Qt5 REQUIRED COMPONENTS Widgets)

pkg_check_modules(KYSDK_QTWIDGETS kysdk-qtwidgets)
include_directories(${KYSDK_QTWIDGETS_INCLUDE_DIRS})
link_directories(${KYSDK_QTWIDGETS_LIBRARY_DIRS})

set(SRCS
        ${EXAMPLE_DIR}/interface/kom_application_interface.cpp
        ${EXAMPLE_DIR}/connector.cpp
        ${EXAMPLE_DIR}/protocol.cpp
        ${EXAMPLE_DIR}/main_widget.cpp)

add_library(${PROJECT_NAME} SHARED ${SRCS})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets)
target_link_libraries(${PROJECT_NAME} PRIVATE ${KYSDK_QTWIDGETS_LIBRARIES})

install(TARGETS ${PROJECT_NAME} DESTINATION /opt/kylin-os-manager/plugins/)

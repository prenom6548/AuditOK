cmake_minimum_required(VERSION 3.14)

project(ukui-notification LANGUAGES CXX)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (BUILD_TEST)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
endif ()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(GNUInstallDirs)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core LinguistTools DBus Network Gui Qml REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core LinguistTools DBus Network Gui Qml REQUIRED)
find_package(ukui-quick COMPONENTS platform REQUIRED)
set(REQUIRED_QT_VERSION 5.12.8)

add_subdirectory(notification-server)
add_subdirectory(libukui-notification)
add_subdirectory(notification-ukcc-plugin)
if(BUILD_TEST)
    add_subdirectory(test)
endif()


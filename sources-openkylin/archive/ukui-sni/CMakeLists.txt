cmake_minimum_required(VERSION 3.14)

project(ukui-sni VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core DBus Network Gui Widgets Quick REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core DBus Network Gui Widgets Quick REQUIRED)

if (QT_VERSION_MAJOR EQUAL 5)
    find_package(Qt5 COMPONENTS X11Extras REQUIRED)
endif()

add_subdirectory(status-notifier-watcher)
add_subdirectory(status-notifier-host)
add_subdirectory(ukui-system-tray)
add_subdirectory(xembed-sni-proxy)
if(BUILD_TEST)
    add_subdirectory(test)
endif()

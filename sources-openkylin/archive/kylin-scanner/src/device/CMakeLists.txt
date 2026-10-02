cmake_minimum_required(VERSION 3.16)
project(device LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")
set(prj_name ${PROJECT_NAME})
find_package(QT NAMES Qt6 Qt5)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools Network DBus REQUIRED)

include_directories(
        ${PROJECT_SOURCE_DIR}/avahi_qt/
        ${PROJECT_SOURCE_DIR}/qt_zeroconf/
        )

set(SRC
        ./qt_zeroconf/zconfserviceclient.h
        ./qt_zeroconf/zconfservice.h
        ./qt_zeroconf/zconfservicebrowser.h
        ./deviceFinder.h
        ./avahi_qt/qt_watch.h
        ./singleton.h
        ./connect.h
        ./snmpFinder.h
        ./network_device_detector.h
        ./base_info.h
        ./common.h
        ./device_information.h
        ./ukui_apt.h
        ./snmpwalk_browser.h
        ./usbFinder.h

        ./device_information.cpp
        ./common.cpp
        ./qt_zeroconf/zconfservicebrowser.cpp
        ./qt_zeroconf/zconfservice.cpp
        ./qt_zeroconf/zconfserviceclient.cpp
        ./network_device_detector.cpp
        ./base_info.cpp
        ./avahi_qt/qt_watch.cpp
        ./deviceFinder.cpp
        ./snmpwalk_browser.cpp
        ./connect.cpp
        ./usbFinder.cpp
        ./snmpFinder.cpp
        ./ukui_apt.cpp
        ./main.cpp

        )

add_executable(${PROJECT_NAME} ${SRC})

target_link_libraries(${PROJECT_NAME} Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::DBus)
target_link_libraries(${prj_name} "-lusb-1.0")
target_link_libraries(${prj_name} "-ludev")
target_link_libraries(${prj_name} "-lcups")
target_link_libraries(${PROJECT_NAME} "-lpthread -lz")
target_link_libraries(${PROJECT_NAME} "-lbsd")

find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB REQUIRED glib-2.0)
include_directories(${GLIB_INCLUDE_DIRS})
target_link_libraries(${prj_name} "-lglib-2.0")

find_library(NETSNMPAGENT "netsnmpagent")
find_library(NETSNMPMIBS "netsnmpmibs")
find_library(NETSNMP "netsnmp")
target_link_libraries(${prj_name} ${NETSNMPAGENT} ${NETSNMPMIBS} ${NETSNMP})

target_link_libraries(${prj_name} "-lavahi-common -lavahi-client")

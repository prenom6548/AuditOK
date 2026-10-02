cmake_minimum_required(VERSION 3.5)
# Set Project Name
project(test_networkdevicebrowse LANGUAGES C CXX)
add_definitions(-DREMOTE_PRINTER)
if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB REQUIRED glib-2.0)
include_directories(${GLIB_INCLUDE_DIRS})
find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets X11Extras Network DBus REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets X11Extras Network DBus REQUIRED)


set (SOURCE_PATH ../..)
#add include_directories
include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/${SOURCE_PATH}/avahi_qt/
    ${PROJECT_SOURCE_DIR}/${SOURCE_PATH}/printer_manager/
    ${PROJECT_SOURCE_DIR}/${SOURCE_PATH}/qt_zeroconf/
    ${PROJECT_SOURCE_DIR}/${SOURCE_PATH}/util/
    ${PROJECT_SOURCE_DIR}/${SOURCE_PATH}/common/
)

file(GLOB ${PROJECT_NAME}_SRC
    "${SOURCE_PATH}/avahi_qt/*"
    "${SOURCE_PATH}/printer_manager/*"
    "${SOURCE_PATH}/qt_zeroconf/*"
    "${SOURCE_PATH}/util/*"
    "${SOURCE_PATH}/common/*"
    "main.cpp"
    "test.*"
)

message(${test_networkdevicebrowse_SRC})
add_executable(${PROJECT_NAME}
    ${${PROJECT_NAME}_SRC}
)

find_library(NETSNMPAGENT "netsnmpagent")
find_library(NETSNMPMIBS "netsnmpmibs")
find_library(NETSNMP "netsnmp")

target_link_libraries(${PROJECT_NAME} Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::X11Extras Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::DBus)

#dependencies
target_link_libraries(${PROJECT_NAME} "-lpthread -lz")
#cups
target_link_libraries(${PROJECT_NAME} "-lcups")
target_link_libraries(${PROJECT_NAME} "-lusb-1.0")

target_link_libraries(${PROJECT_NAME} "-ludev")
target_link_libraries(${PROJECT_NAME} "-lglib-2.0")
target_link_libraries(${PROJECT_NAME} "-lavahi-common -lavahi-client")

target_link_libraries(${PROJECT_NAME} ${NETSNMPAGENT} ${NETSNMPMIBS} ${NETSNMP})
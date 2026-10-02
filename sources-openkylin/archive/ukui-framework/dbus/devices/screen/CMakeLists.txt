cmake_minimum_required(VERSION 3.5)
project(screen VERSION 1.0)  

find_package(PkgConfig)


pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GIO REQUIRED gio-2.0)
pkg_check_modules(GOBJECT REQUIRED gobject-2.0)

include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${GIO_INCLUDE_DIRS})
include_directories(${GOBJECT_INCLUDE_DIRS})

# 查找Qt5及其模块
find_package(QT NAMES Qt6 Qt5 COMPONENTSCONFIG)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core DBus REQUIRED)


include_directories(${CMAKE_SOURCE_DIR}/dbus/common/include)
include_directories(${CMAKE_SOURCE_DIR}/dbus/common/QGSettings)
include_directories(${CMAKE_SOURCE_DIR}/dbus/)

set(DEVICES_SCREEN_LINK
    glib-2.0
    gio-2.0
    ${GLIB_LIBRARIES}
    ${GIO_LIBRARIES}
    ${GOBJECT_LIBRARIES}
)


# 索引动态库的头文件地址
include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${GIO_INCLUDE_DIRS})
include_directories(${GOBJECT_INCLUDE_DIRS})

set(CMAKE_AUTOMOC ON)

file(GLOB LIBSCREEN_SOURCES src/*.cpp)

add_library(screen STATIC ${LIBSCREEN_SOURCES})  

target_link_libraries(screen
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::DBus
    ${DEVICES_SCREEN_LINK})

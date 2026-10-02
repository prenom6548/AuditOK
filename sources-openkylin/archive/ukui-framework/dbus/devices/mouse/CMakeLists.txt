cmake_minimum_required(VERSION 3.5)
project(mouse VERSION 1.0)  

find_package(PkgConfig)


include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${GIO_INCLUDE_DIRS})
include_directories(${GOBJECT_INCLUDE_DIRS})

# 查找Qt5及其模块


find_package(QT NAMES Qt6 Qt5 COMPONENTSCONFIG REQUIRED Core DBus)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core DBus REQUIRED)


include_directories(${CMAKE_SOURCE_DIR}/dbus/common/include)
include_directories(${CMAKE_SOURCE_DIR}/dbus/common/QGSettings)

set(CMAKE_AUTOMOC ON)
file(GLOB LIBMOUSE_SOURCES src/*.cpp)

add_library(mouse STATIC ${LIBMOUSE_SOURCES})  

target_link_libraries(mouse
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::DBus)

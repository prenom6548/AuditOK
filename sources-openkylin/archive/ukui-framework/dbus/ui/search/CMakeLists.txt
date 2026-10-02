cmake_minimum_required(VERSION 3.14)

project(configSearch LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core DBus REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core DBus REQUIRED)

pkg_check_modules(GIO REQUIRED gio-2.0)

include_directories(
    ${GIO_INCLUDE_DIRS}
)
include_directories(${CMAKE_SOURCE_DIR}/dbus/common/QGSettings)

add_library(configSearch STATIC
    search-config.cpp
    search-config.h
)

target_link_libraries(configSearch PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::DBus gio-2.0 ${GIO_LIBRARIES})

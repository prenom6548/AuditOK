cmake_minimum_required(VERSION 3.14)

project(theme LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
find_package(QT NAMES Qt6 Qt5 COMPONENTS)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Gui DBus REQUIRED)

pkg_check_modules(GIO REQUIRED gio-2.0)


include_directories(
    ${GIO_INCLUDE_DIRS}
    ${QGSETTINGS_INCLUDE_DIRS}
)
include_directories(${CMAKE_SOURCE_DIR}/dbus/common/include)
include_directories(${CMAKE_SOURCE_DIR}/dbus/common/QGSettings)

add_library(theme STATIC
  theme.cpp
  theme.h
)

target_link_libraries(theme PRIVATE Qt${QT_VERSION_MAJOR}::Gui Qt${QT_VERSION_MAJOR}::DBus gio-2.0 ${QGSETTINGS_LIBRARIES})

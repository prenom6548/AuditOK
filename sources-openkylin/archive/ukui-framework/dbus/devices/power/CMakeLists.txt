cmake_minimum_required(VERSION 3.5)

project(power VERSION 1.0)

find_package(PkgConfig)
set(CMAKE_AUTOMOC ON)

file(GLOB LIBPOWER_SOURCES src/*.cpp)

find_package(QT NAMES Qt6 Qt5 COMPONENTSCONFIG)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core DBus REQUIRED)
add_library(power STATIC ${LIBPOWER_SOURCES})

find_package(PkgConfig REQUIRED)

pkg_check_modules(gsettings-qt REQUIRED IMPORTED_TARGET gsettings-qt)

target_link_libraries(power PRIVATE PkgConfig::gsettings-qt)

target_link_libraries(power
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    ${DEVICES_POWER_LINK})

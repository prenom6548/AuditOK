cmake_minimum_required(VERSION 3.5)
project(bluetooth VERSION 1.0)  

find_package(PkgConfig)

cmake_minimum_required(VERSION 3.16)
# 查找Qt5及其模块
find_package(QT NAMES Qt6 Qt5 COMPONENTSCONFIG)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core DBus REQUIRED)

set(CMAKE_AUTOMOC ON)

file(GLOB LIBBLUETOOTH_SOURCES src/*.cpp)

add_library(bluetooth STATIC ${LIBBLUETOOTH_SOURCES})  

target_link_libraries(bluetooth
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus)

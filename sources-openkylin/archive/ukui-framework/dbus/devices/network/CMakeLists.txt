cmake_minimum_required(VERSION 3.5)
project(network VERSION 1.0)  

find_package(PkgConfig)

cmake_minimum_required(VERSION 3.16)
# 查找Qt5及其模块
find_package(QT NAMES Qt6 Qt5 COMPONENTSCONFIG)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core DBus REQUIRED)

# 索引动态库的头文件地址
include_directories(${GOBJECT_INCLUDE_DIRS})

set(CMAKE_AUTOMOC ON)

file(GLOB LIBNETWORK_SOURCES src/*.cpp)

add_library(network STATIC ${LIBNETWORK_SOURCES})  

target_link_libraries(network
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
)

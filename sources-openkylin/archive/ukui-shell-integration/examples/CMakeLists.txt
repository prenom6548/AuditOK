cmake_minimum_required(VERSION 3.16)

project(examples)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS
    Widgets)

add_executable(qtdemo main.cpp widget.cpp)

target_link_libraries(qtdemo
    Qt${QT_VERSION_MAJOR}::Widgets)
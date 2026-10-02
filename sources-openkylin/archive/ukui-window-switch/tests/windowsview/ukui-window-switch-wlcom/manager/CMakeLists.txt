cmake_minimum_required(VERSION 3.16)

project(test_manager)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC --coverage")

find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Core Quick Qml Test REQUIRED)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../windowsview/ukui-window-switch-wlcom)
include_directories(${SOURCE_DIR})

add_executable(test_manager
    main.cpp
    test_manager.cpp
    test_manager.h
    ${SOURCE_DIR}/manager.cpp
)

target_link_libraries(test_manager
    PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Quick
    Qt5::Qml
    Qt5::Test
)

target_compile_definitions(test_manager PRIVATE QT_QML_DEBUG)

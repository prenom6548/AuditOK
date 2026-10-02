cmake_minimum_required(VERSION 3.16)
project(test_desktopmanager)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage -lgcov")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets Core Quick Test DBus Gui)
find_package(PkgConfig REQUIRED)
find_package(ukui-quick COMPONENTS platform core REQUIRED)
pkg_check_modules(QGSettings REQUIRED gsettings-qt)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../windowsview)
include_directories(
    ${SOURCE_DIR}
    ${QGSettings_INCLUDE_DIRS}
)

set(TEST_SOURCES
    test_desktopmanager.cpp
    test_desktopmanager.h
    main.cpp
    ${SOURCE_DIR}/desktopmanager.cpp
    ${SOURCE_DIR}/desktopmanager.h
    ${SOURCE_DIR}/abstractdesktopmanager.h
    ${SOURCE_DIR}/screens-manager.cpp
    ${SOURCE_DIR}/screens-manager.h
    ${SOURCE_DIR}/ukui-window-switch-wlcom/kywcdesktopmanager.cpp
    ${SOURCE_DIR}/ukui-window-switch-wlcom/kywcdesktopmanager.h
)

add_executable(test_desktopmanager ${TEST_SOURCES})

target_link_libraries(test_desktopmanager
    Qt5::Widgets
    Qt5::Core
    Qt5::Quick
    Qt5::Test
    Qt5::DBus
    Qt5::Gui
    ${QGSettings_LIBRARIES}
    ukui-quick::platform
    ukui-quick::core
)

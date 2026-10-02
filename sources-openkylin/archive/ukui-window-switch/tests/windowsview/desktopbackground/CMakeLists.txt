cmake_minimum_required(VERSION 3.16)
project(test_desktopbackground)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage -lgcov")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets Core Quick Test DBus)
find_package(PkgConfig REQUIRED)
pkg_check_modules(QGSettings REQUIRED gsettings-qt)
find_package(KF5CoreAddons REQUIRED)
find_package(KF5WindowSystem REQUIRED)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../windowsview)
include_directories(
    ${SOURCE_DIR}
    ${QGSettings_INCLUDE_DIRS}
    ${KF5CoreAddons_INCLUDE_DIRS}
    ${KF5WindowSystem_INCLUDE_DIRS}
)

set(TEST_SOURCES
    test_desktopbackground.cpp
    test_desktopbackground.h
    main.cpp
    ${SOURCE_DIR}/desktopbackground.cpp
    ${SOURCE_DIR}/desktopbackground.h
)

add_executable(test_desktopbackground ${TEST_SOURCES})

target_link_libraries(test_desktopbackground
    Qt5::Widgets
    Qt5::Core
    Qt5::Quick
    Qt5::Test
    Qt5::DBus
    Qt5::QuickPrivate
    ${QGSettings_LIBRARIES}
    KF5::CoreAddons
    KF5::WindowSystem
)

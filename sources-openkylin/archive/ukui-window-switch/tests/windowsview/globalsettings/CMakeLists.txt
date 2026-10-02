cmake_minimum_required(VERSION 3.16)

project(test_globalsettings)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage -lgcov")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(QGSettings REQUIRED gsettings-qt)

find_package(Qt5 REQUIRED COMPONENTS Core Test)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../windowsview)
include_directories(
    ${SOURCE_DIR}
    ${QGSettings_INCLUDE_DIRS}
)

set(TEST_SOURCES
    test_globalsettings.cpp
    test_globalsettings.h
    main.cpp
    ${SOURCE_DIR}/globalsettings.cpp
    ${SOURCE_DIR}/globalsettings.h
)

add_executable(test_globalsettings ${TEST_SOURCES})

target_link_libraries(test_globalsettings
    Qt5::Core
    Qt5::Test
    ${QGSettings_LIBRARIES}
)


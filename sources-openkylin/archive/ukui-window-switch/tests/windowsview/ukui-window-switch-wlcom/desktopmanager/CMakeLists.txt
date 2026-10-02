cmake_minimum_required(VERSION 3.16)

project(test_kywcdesktopmanager)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage -lgcov")

find_package(Qt5 REQUIRED COMPONENTS Core Quick Test)
find_package(PkgConfig REQUIRED)
find_package(ukui-quick COMPONENTS platform core REQUIRED)

pkg_check_modules(KYWC REQUIRED kywc)
pkg_check_modules(XDG REQUIRED Qt5Xdg)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../windowsview/ukui-window-switch-wlcom)
set(TEST_SOURCE_DIR ${CMAKE_SOURCE_DIR})

include_directories(${SOURCE_DIR})
include_directories(${SOURCE_DIR}/..)
include_directories(${TEST_SOURCE_DIR})

include_directories(${KYWC_INCLUDE_DIRS})
include_directories(${XDG_INCLUDE_DIRS})

include_directories(${Qt5Quick_PRIVATE_INCLUDE_DIRS})
include_directories(${ukui-quick_INCLUDE_DIRS})

add_executable(test_kywcdesktopmanager
    main.cpp
    test_kywcdesktopmanager.cpp
    test_kywcdesktopmanager.h
    ${SOURCE_DIR}/kywcdesktopmanager.cpp
    ${SOURCE_DIR}/../abstractdesktopmanager.h
    ${SOURCE_DIR}/../screens-manager.cpp)

target_link_libraries(test_kywcdesktopmanager
    Qt5::Core
    Qt5::Test
    ${KYWC_LIBRARIES}
    ${XDG_LIBRARIES}
    ukui-quick::platform
    ukui-quick::core
    wayland-client
)

target_compile_definitions(test_kywcdesktopmanager PRIVATE QT_QML_DEBUG)

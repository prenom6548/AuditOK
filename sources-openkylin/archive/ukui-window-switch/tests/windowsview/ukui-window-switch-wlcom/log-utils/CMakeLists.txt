cmake_minimum_required(VERSION 3.16)

project(test_log_utils)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lgcov")

find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Core Test)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../windowsview/ukui-window-switch-wlcom)
include_directories(${SOURCE_DIR})

set(TEST_SOURCES
    main.cpp
    test_log-utils.cpp
    test_log-utils.h
    ${SOURCE_DIR}/log-utils.cpp
)

add_executable(test_log_utils ${TEST_SOURCES})

target_link_libraries(test_log_utils
    PRIVATE
    Qt5::Core
    Qt5::Test
)

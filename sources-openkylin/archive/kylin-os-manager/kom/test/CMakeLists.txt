cmake_minimum_required(VERSION 3.5)

project(kom-test LANGUAGES CXX)

set(KOM_TEST_DIR ${CMAKE_CURRENT_LIST_DIR})

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_FLAGS "-Wall -g")
set(CMAKE_CXX_STANDARD 17)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin/)

add_subdirectory(${KOM_TEST_DIR}/../ ${CMAKE_BINARY_DIR}/kom/)

include_directories(${KOM_TEST_DIR})
include_directories(${KOM_TEST_DIR}/../)

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

set(SRCS
	${KOM_TEST_DIR}/test-kom-label.cpp
)

add_executable(test-kom-label ${SRCS})
target_link_libraries(test-kom-label kom)

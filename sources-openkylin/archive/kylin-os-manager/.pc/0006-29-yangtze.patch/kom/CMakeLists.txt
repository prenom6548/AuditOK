cmake_minimum_required(VERSION 3.5)

project(kom)

set(KOM_DIR ${CMAKE_CURRENT_LIST_DIR})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_FLAGS "-Wall -g")
set(CMAKE_CXX_STANDARD 17)

if (CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    add_definitions(-DDEBUG_MODE)
endif()

include_directories(${KOM_DIR})
include_directories(${KOM_DIR}/include)

find_package(PkgConfig)
find_package(Qt5 COMPONENTS Widgets REQUIRED)

pkg_check_modules(GSETTINGS gsettings-qt)
include_directories(${GSETTINGS_INCLUDE_DIRS})
link_directories(${GSETTINGS_LIBARAY_DIRS})

set(SRCS
        ${KOM_DIR}/kom-label.cpp
        ${KOM_DIR}/kom-ukui-gsettings.cpp
        ${KOM_DIR}/kom-utils.cpp
        ${KOM_DIR}/kom-configure.cpp
        ${KOM_DIR}/kom-buriedpoint.cpp
        ${KOM_DIR}/kom-radiuswidget.cpp
        ${KOM_DIR}/security_utils.cpp)

add_library(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME}  Qt5::Widgets)
target_link_libraries(${PROJECT_NAME}  ${GSETTINGS_LIBRARIES})

pkg_check_modules(KYSDK_MODULE kysdk-diagnostics)
include_directories(${KYSDK_MODULE_INCLUDE_DIRS})
link_directories(${KYSDK_MODULE_LIBRARY_DIRS})
link_libraries(${KYSDK_MODULE_LIBRARIES})

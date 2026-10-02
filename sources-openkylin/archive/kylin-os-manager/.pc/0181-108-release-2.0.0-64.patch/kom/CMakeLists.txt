cmake_minimum_required(VERSION 3.5)

project(kom)

set(KOM_DIR ${CMAKE_CURRENT_LIST_DIR})
set(TOP_DIR ${CMAKE_CURRENT_LIST_DIR}/../)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_FLAGS "-Wall -g")
set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig)
find_package(Qt5 COMPONENTS Widgets DBus REQUIRED)

set(SRCS
        ${KOM_DIR}/kom_buriedpoint.cpp
        ${KOM_DIR}/kom-label.cpp
        ${KOM_DIR}/kom-ukui-gsettings.cpp
        ${KOM_DIR}/kom-utils.cpp
        ${KOM_DIR}/kom-configure.cpp
        ${KOM_DIR}/kom-radiuswidget.cpp
        ${KOM_DIR}/kom-service.cpp
        ${KOM_DIR}/kom-scorewidget.cpp
        ${KOM_DIR}/security_utils.cpp
        )

include_directories(${KOM_DIR})

add_library(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::DBus)
target_link_libraries(${PROJECT_NAME} PRIVATE dl)

pkg_check_modules(GSETTINGS REQUIRED gsettings-qt)
include_directories(${GSETTINGS_INCLUDE_DIRS})
link_directories(${GSETTINGS_LIBARAY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${GSETTINGS_LIBRARIES})

pkg_check_modules(KYSDK_DATACOLLECT kysdk-datacollect)
if (${KYSDK_DATACOLLECT_FOUND})
    include_directories(${KYSDK_DATACOLLECT_INCLUDE_DIRS})
    link_directories(${KYSDK_DATACOLLECT_LIBRARY_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${KYSDK_DATACOLLECT_LIBRARIES})
else ()
    include_directories(${TOP_DIR}/compatibility/kysdk-datacollect/)
    target_link_libraries(${PROJECT_NAME} PRIVATE compatibility)
endif ()

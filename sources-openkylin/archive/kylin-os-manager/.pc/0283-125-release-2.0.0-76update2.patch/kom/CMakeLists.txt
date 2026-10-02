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
find_package(KF5WindowSystem REQUIRED)

if (V11_OPTION)
    set(SRCS
        ${KOM_DIR}/kom_process.cpp
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
else ()
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
endif ()

include_directories(${KOM_DIR})

add_library(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::DBus)
target_link_libraries(${PROJECT_NAME} PRIVATE KF5::WindowSystem)
target_link_libraries(${PROJECT_NAME} PRIVATE dl)

pkg_check_modules(GSETTINGS IMPORTED_TARGET REQUIRED gsettings-qt)
target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::GSETTINGS)

pkg_check_modules(KYSDK IMPORTED_TARGET REQUIRED kysdk-datacollect kysdk-log kysdk-package kysdk-waylandhelper kysdk-alm)
target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::KYSDK)

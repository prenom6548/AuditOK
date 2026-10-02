cmake_minimum_required(VERSION 3.16)

project(osdServer LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTSCONFIG)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Gui Core Quick DBus REQUIRED)

find_package(KF5WindowSystem REQUIRED)

#pkg_check_modules(KYSDK REQUIRED IMPORTED_TARGET kysdk-waylandhelper kysdk-ukuiwindowhelper)

set(UKUI_OSD_EXTERNAL_LIBS "")
set(UKUI_OSD_PC_PKGS kysdk-waylandhelper kysdk-ukuiwindowhelper)

foreach(PC_LIB IN ITEMS ${UKUI_OSD_PC_PKGS})
    pkg_check_modules(${PC_LIB} REQUIRED IMPORTED_TARGET ${PC_LIB})
    if(${${PC_LIB}_FOUND})
        include_directories(${${PC_LIB}_INCLUDE_DIRS})
        link_directories(${${PC_LIB}_LIBRARY_DIRS})
        list(APPEND UKUI_OSD_EXTERNAL_LIBS PkgConfig::${PC_LIB})
    endif()
endforeach()

include_directories(${KYSDK_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/dbus/common/include)
include_directories(${CMAKE_SOURCE_DIR}/dbus/common/QGSettings)

add_library(osdServer
    ${RCC_SOURCES}
    osdserver.cpp
    osdserver.h
    qmlmanager.cpp
    qmlmanager.h
    datamanager.cpp
    datamanager.h
    osd_string.h
    osdbasicwindow.cpp
    osdbasicwindow.h
    qml.qrc
    )

target_link_libraries(osdServer
  PRIVATE
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::Quick
  Qt${QT_VERSION_MAJOR}::DBus
  Qt${QT_VERSION_MAJOR}::Gui
  ${UKUI_OSD_EXTERNAL_LIBS})

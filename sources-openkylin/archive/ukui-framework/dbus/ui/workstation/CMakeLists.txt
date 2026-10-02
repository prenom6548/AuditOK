cmake_minimum_required(VERSION 3.10)
project(configWorkstation VERSION 1.0)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# 查找Qt5及其模块
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core DBus RemoteObjects REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core DBus RemoteObjects REQUIRED)

find_package(PkgConfig)

pkg_check_modules(QGSettings REQUIRED gsettings-qt)
include_directories(${QGSETTINGS_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/dbus/common/QGSettings)

#set(WORKSTATION_EXTERNAL_LIBS "")
#set(WORKSTATION_PC_PKGS gsettings-qt)

#foreach(PC_LIB IN ITEMS ${WORKSTATION_PC_PKGS})
#    pkg_check_modules(${PC_LIB} REQUIRED IMPORTED_TARGET ${PC_LIB})
#    if(${${PC_LIB}_FOUND})
#        include_directories(${${PC_LIB}_INCLUDE_DIRS})
#        link_directories(${${PC_LIB}_LIBRARY_DIRS})
#        list(APPEND WORKSTATION_EXTERNAL_LIBS PkgConfig::${PC_LIB})
#    endif()
#endforeach()

set(CONFIG_WORKSTATION_SOURCES
    workstation.cpp
    workstation.h
)


add_library(configWorkstation STATIC ${CONFIG_WORKSTATION_SOURCES})

target_link_libraries(configWorkstation
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::RemoteObjects
    ${QGSETTINGS_LIBRARIES}
#    ${WORKSTATION_EXTERNAL_LIBS}
)

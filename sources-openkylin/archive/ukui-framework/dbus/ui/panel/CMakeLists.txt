cmake_minimum_required(VERSION 3.5)
project(configUkpanel VERSION 1.0)

find_package(PkgConfig)

pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GIO REQUIRED gio-2.0)
pkg_check_modules(GOBJECT REQUIRED gobject-2.0)

# 索引动态库的头文件地址
include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${GIO_INCLUDE_DIRS})
include_directories(${GOBJECT_INCLUDE_DIRS})
# add_subdirectory(test)

# 查找Qt5及其模块
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core DBus RemoteObjects REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core DBus RemoteObjects REQUIRED)
include_directories(src)
include_directories(panel_include)
set(CONFIG_PANEL_LINK
    glib-2.0
    gio-2.0
    ${GLIB_LIBRARIES}
    ${GIO_LIBRARIES}
    ${GOBJECT_LIBRARIES}
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(LIBCONFIG_PANEL_SOURCES
    src/panel_config.cpp
    src/panel_config.h
    panel_include/general_config_define.h
)

# 生成远程对象代码
qt5_generate_repc(LIBCONFIG_PANEL_SOURCES panel_include/remote_config.rep REPLICA)


add_library(configUkpanel STATIC ${LIBCONFIG_PANEL_SOURCES})
message("CMAKE_CURRENT_BINARY_DIR" ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(configUkpanel PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(configUkpanel BEFORE PRIVATE  panel_include)

target_link_libraries(configUkpanel
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::RemoteObjects
    ${CONFIG_PANEL_LINK})

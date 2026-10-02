cmake_minimum_required(VERSION 3.16)
project(test_config_panel)

find_package(PkgConfig)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GIO REQUIRED gio-2.0)
pkg_check_modules(GOBJECT REQUIRED gobject-2.0)

include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${GIO_INCLUDE_DIRS})
include_directories(${GOBJECT_INCLUDE_DIRS})


# 查找Qt5及其模块
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core DBus RemoteObjects REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core DBus RemoteObjects REQUIRED)
include_directories(../src)

# 查找Qt5及其模块
find_package(Qt5 COMPONENTS Core Widgets DBus REQUIRED Test)  
  
# 接下来，你可以使用Qt5::Core, Qt5::Widgets, Qt5::DBus来链接你的可执行文件或库  
set(CMAKE_AUTOMOC ON)
include_directories(${CMAKE_CURRENT_BINARY_DIR})


set(LIBCONFIG_PANEL_SOURCES
    test.cpp
    test.h
    ../src/panel_config.h
    ../src/panel_config.cpp
    ../panel_include/general_config_define.h
)
# 生成远程对象代码
qt5_generate_repc(LIBCONFIG_PANEL_SOURCES ../panel_include/remote_config.rep REPLICA)
# 示例：添加可执行文件  
add_executable(test_config_panel
    ${LIBCONFIG_PANEL_SOURCES}
)



# 链接Qt模块  
target_link_libraries(test_config_panel
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::RemoteObjects
    ${GLIB_LIBRARIES}
    ${GIO_LIBRARIES}
    ${GOBJECT_LIBRARIES})

enable_testing()
add_test(NAME ConfigPanelTest COMMAND config_panel)

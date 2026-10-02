cmake_minimum_required(VERSION 3.5)
find_package(PkgConfig)
project(ukui-framework-dbus VERSION 1.0)

find_package(QT NAMES Qt6 Qt5 COMPONENTS)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core DBus Widgets REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(devices)
add_subdirectory(ui)
add_subdirectory(navigations)
add_subdirectory(config)
add_subdirectory(notifications)

aux_source_directory(SOURCES devices/README.md)
aux_source_directory(SOURCES ui/README.md)
aux_source_directory(SOURCES data/)

include_directories(${CMAKE_INCLUDE_CURRENT_DIR})
include_directories(devices/screen/src)
include_directories(devices/audio/src)
include_directories(devices/network/src)
include_directories(devices/bluetooth/src)
include_directories(devices/power/src)
include_directories(devices/touchpad/src)
include_directories(devices/keyboard/src)
include_directories(devices/mouse/src)

include_directories(ui/panel/src)
include_directories(ui/panel/panel_include)
include_directories(ui/background)
include_directories(ui/effect)
include_directories(ui/font)
include_directories(ui/theme)
include_directories(ui/panel)
include_directories(ui/osdServer)
include_directories(ui/datetime)
include_directories(ui/workstation)
include_directories(ui/search)

include_directories(config/account)

include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${GIO_INCLUDE_DIRS})
include_directories(${GOBJECT_INCLUDE_DIRS})

include_directories(navigations/ukui_navigations/src)
include_directories(navigations/shortcut_navigations/src)
include_directories(notifications)

include_directories(${CMAKE_SOURCE_DIR}/dbus/common/include)
include_directories(${CMAKE_SOURCE_DIR}/dbus/common/QGSettings)

add_executable(ukui-framework-dbus
  main.cpp
  ${CMAKE_SOURCE_DIR}/dbus/common/include/gsettings_manager.cpp
  ${CMAKE_SOURCE_DIR}/dbus/common/include/gsettings_manager.h
  ${CMAKE_SOURCE_DIR}/dbus/common/QGSettings/qgsettings.cpp
  ${CMAKE_SOURCE_DIR}/dbus/common/QGSettings/qgsettings.h
  ${CMAKE_SOURCE_DIR}/dbus/common/QGSettings/qconftype.cpp
  ${CMAKE_SOURCE_DIR}/dbus/common/QGSettings/qconftype.h
)

target_link_libraries(ukui-framework-dbus
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::DBus
    screen
    audio
    network
    bluetooth
    power
    touchpad
    keyboard
    mouse
    theme
    background
    font
    effect
    configUkpanel
    ukuinavigations
    shortcutnavigations
    osdServer
    account
    datetime
    notifications
    configWorkstation
    configSearch
)

install(TARGETS ${PROJECT_NAME}  DESTINATION /usr/bin)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../data/ukui-framework-dbus.desktop
    DESTINATION /etc/xdg/autostart)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../data/org.ukui.Framework.service
    DESTINATION /usr/share/dbus-1/services)

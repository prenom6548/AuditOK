cmake_minimum_required(VERSION 3.16)

# 调试多任务视图需要取消注释下面
# add_definitions("-Wall -g")
set(PLUGIN_NAME "windowsview")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(KF5_DEP_VERSION "5.54.0")

find_package(ECM 5.54.0 NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

find_package(OpenGL)
find_package(EGL MODULE)
find_package(KF5Config REQUIRED)
find_package(KF5GlobalAccel REQUIRED)
find_package(KF5I18n REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Qt5LinguistTools REQUIRED)
find_package(Qt5 REQUIRED NO_MODULE COMPONENTS X11Extras Concurrent Quick Qml DBus)
find_package(ukui-quick COMPONENTS platform core)
find_package(ukui-search REQUIRED)
find_package(Qt5Xdg REQUIRED)

pkg_check_modules(KDK REQUIRED kysdk-sysinfo)
pkg_check_modules(DATACOLLECT REQUIRED kysdk-datacollect)

include_directories(${KDK_INCLUDE_DIRS})
include_directories(${DATACOLLECT_INCLUDE_DIRS})

add_definitions(-DKWINEFFECT=1)

set(SRCS    ../desktopbackground.cpp
            ../windowmanagerinterface.cpp
            multitaskviewmanager.cpp
            dbusservice.cpp
            multitaskviewmanagerpluginfactory.cpp
            ../desktoplistmodel.h
            ../desktoplistmodel.cpp
            ../appwindowlistmodel.h
            ../appwindowlistmodel.cpp
            ../appwindowfiltermodel.h
            ../appwindowfiltermodel.cpp
            ../ukui-flow.cpp
            ../ukui-flow.h
            ../screens-manager.cpp
            ../screens-manager.h
            ../abstractdesktopmanager.h
            ../desktopmanager.h
            ../desktopmanager.cpp
            kwindesktopmanager.h
            kwindesktopmanager.cpp
            ../windowsmanager.h
            ../windowsmanager.cpp
 )

qt5_add_resources(SRCS ../data.qrc)
add_library(
    ${PLUGIN_NAME} MODULE
    ${SRCS}
)

target_link_libraries(
        ${PLUGIN_NAME}
        PRIVATE
        Qt5::Core
        Qt5::Gui
        Qt5::Quick
        Qt5::QuickPrivate
        KF5::ConfigCore
        KF5::WindowSystem
        KF5::CoreAddons
        KF5::GlobalAccel
        Qt5::X11Extras
        Qt5::Concurrent
        Qt5::DBus
        Qt5::Qml
        Qt::QmlPrivate
        ${QGSettings_LIBRARIES}
        ${KDK_LINK_LIBRARIES}
        ${DATACOLLECT_LINK_LIBRARIES}
        ukui-quick::platform
        ukui-quick::core
        ukui-search
        Qt5Xdg
)
set(INSTALL_PATH ${QT_INSTALL_PLUGINS}/kwin/effects/plugins/)
install(TARGETS ${PLUGIN_NAME} DESTINATION ${INSTALL_PATH})

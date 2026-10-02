cmake_minimum_required(VERSION 3.5)

project(ukui-volume-control-applet-tray LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Widgets DBus LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Widgets DBus LinguistTools X11Extras REQUIRED)
find_package(Qt5 COMPONENTS Widgets DBus REQUIRED)
find_package(KF5WindowSystem REQUIRED)

pkg_check_modules(X11 REQUIRED x11)
pkg_check_modules(KYSDK REQUIRED kysdk-qtwidgets)

include_directories(${X11_INCLUDE_DIRS})
include_directories(${KYSDK_INCLUDE_DIRS})

file(GLOB TS_FILES translations/*.ts)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${TS_FILES} OPTIONS -no-obsolete -no-ui-lines)
    qt_add_executable(${PROJECT_NAME}
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${QRC_FILES}
        ${QM_FILES}
        )

else()
    qt5_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${TS_FILES} OPTIONS -no-obsolete -no-ui-lines)
    add_executable(ukui-volume-control-applet-tray
        main.cpp
        ISystemVolumeWidget.h
        IAppVolumeWidget.h
        ITrayVolumeSliderItem.cpp
        ITrayVolumeSliderItem.h
        IDevicePortSelectItem.h
        Ukui4DevicePortSelectItem.cpp
        Ukui4DevicePortSelectItem.h
        Ukui4SystemVolumeSliderItem.cpp
        Ukui4SystemVolumeSliderItem.h
        TrayClientManager.cpp
        TrayClientManager.h
        TrayClientMethod.cpp
        TrayClientMethod.h
        ukui4.0/Ukui4SystemVolumeWidget.cpp
        ukui4.0/Ukui4SystemVolumeWidget.h
        ukui4.0/Ukui4AppVolumeWidget.cpp
        ukui4.0/Ukui4AppVolumeWidget.h
        ukui4.0/TrayListViewItemModel.cpp
        ukui4.0/TrayListViewItemModel.h
        ukui4.0/TrayListViewDelegate.cpp
        ukui4.0/TrayListViewDelegate.h
        IMainWindow.cpp
        IMainWindow.h
        Ukui2MainWindow.cpp
        Ukui2MainWindow.h
        Ukui2MiniWidget.cpp
        Ukui2MiniWidget.h
        Ukui2SystemVolumeWidget.cpp
        Ukui2SystemVolumeWidget.h
        Ukui2AppVolumeWidget.cpp
        Ukui2AppVolumeWidget.h
        Ukui2SystemVolumeSliderItem.cpp
        Ukui2SystemVolumeSliderItem.h
        ukui4.0/Ukui4MainWindow.cpp
        ukui4.0/Ukui4MainWindow.h
        UkmediaMainWidget.cpp
        UkmediaMainWidget.h
        UkmediaTrayIcon.cpp
        UkmediaTrayIcon.h
        Ukui2TrayIcon.cpp
        Ukui2TrayIcon.h
        Ukui4TrayIcon.cpp
        Ukui4TrayIcon.h
        XatomHelper.cpp
        XatomHelper.h
        ../common/DBusClient.cpp
        ../common/DBusClient.h
        ../common/Ukui4CustomControl.cpp
        ../common/Ukui4CustomControl.h
        )

endif()

target_include_directories(ukui-volume-control-applet-tray
    PUBLIC ../backend ../common
    )

target_link_libraries(ukui-volume-control-applet-tray PRIVATE
    Qt5::Widgets
    Qt5::DBus
    Qt5::X11Extras
    KF5::WindowSystem
    ${X11_LIBRARIES}
    ${KYSDK_LIBRARIES}
    )

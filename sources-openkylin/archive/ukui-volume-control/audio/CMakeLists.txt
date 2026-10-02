cmake_minimum_required(VERSION 3.5)

project(audio_U5 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Widgets Gui DBus LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Widgets Gui DBus LinguistTools REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(GLIB2 REQUIRED glib-2.0 gio-2.0)
pkg_check_modules(KYSDK REQUIRED kysdk-qtwidgets)
pkg_check_modules(PA REQUIRED libpulse libpulse-mainloop-glib)
pkg_check_modules(QGSETTING REQUIRED gsettings-qt)
pkg_check_modules(LibCanberra REQUIRED IMPORTED_TARGET libcanberra)

include_directories(${GLIB2_INCLUDE_DIRS})
include_directories(${KYSDK_INCLUDE_DIRS})
include_directories(${QGSETTING_INCLUDE_DIRS})
include_directories(resources/)

set(RESOURCES resources/resources.qrc)

file(GLOB TS_FILES translations/*.ts)

# Build only runs lrelease; do not lupdate .ts (avoids parallel write races).
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_translation(QM_FILES ${TS_FILES})
    qt_add_executable(${PROJECT_NAME}
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${QRC_FILES}
        ${QM_FILES}
        ${RESOURCES}
        )

else()
    qt5_add_translation(QM_FILES ${TS_FILES})
    add_library(audio_testu2 SHARED
        audio.ui
        customstyle.cpp
        customstyle.h
        ClientMethod.cpp
        ClientMethod.h
        IVolumeSliderItem.cpp
        IVolumeSliderItem.h
        ISelectComboxItem.cpp
        ISelectComboxItem.h
        ISwitchButtonItem.h
        ISwitchButtonItem.cpp
        IAudioMainWidow.cpp
        IAudioMainWidow.h
        ClientManager.cpp
        ClientManager.h
        MonitorStream.cpp
        MonitorStream.h
        include/interface.h
        include/SwitchButton.cpp
        include/SwitchButton.h
        MainWidget.cpp
        MainWidget.h
        ConcreteStrategy.cpp
        ConcreteStrategy.h
        TitleLabelItem.cpp
        TitleLabelItem.h
        Ukui4MainWidget.cpp
        Ukui4MainWidget.h
        Ukcc5DevicePortItem.cpp
        Ukcc5DevicePortItem.h
        Ukui5MainWidget.cpp
        Ukui5MainWidget.h
        ukui4.0/Ukui4SwitchButtonItem.cpp
        ukui4.0/Ukui4SwitchButtonItem.h
        ukui4.0/Ukui4VolumeSliderItem.cpp
        ukui4.0/Ukui4VolumeSliderItem.h
        ukui4.0/Ukui4SelectComboxItem.cpp
        ukui4.0/Ukui4SelectComboxItem.h
        ukui2.0/Ukcc2MainWidget.cpp
        ukui2.0/Ukcc2MainWidget.h
        ukui2.0/Ukui2SwitchButtonItem.cpp
        ukui2.0/Ukui2SwitchButtonItem.h
        ukui2.0/Ukui2VolumeSliderItem.cpp
        ukui2.0/Ukui2VolumeSliderItem.h
        ukui2.0/Ukui2SelectComboxItem.cpp
        ukui2.0/Ukui2SelectComboxItem.h
        ukui2.0/AudioPulginUkui2.cpp
        ukui2.0/AudioPulginUkui2.h
        ../common/Ukui4CustomControl.cpp
        ../common/Ukui4CustomControl.h
        ../common/DBusClient.cpp
        ../common/DBusClient.h
        )

    add_library(audio_U5 SHARED
        audio.ui
        main.cpp
        customstyle.cpp
        customstyle.h
        ClientMethod.cpp
        ClientMethod.h
        IDetailSettingsItem.cpp
        IDetailSettingsItem.h
        IVolumeSliderItem.cpp
        IVolumeSliderItem.h
        ISelectComboxItem.cpp
        ISelectComboxItem.h
        ISwitchButtonItem.h
        ISwitchButtonItem.cpp
        IAudioMainWidow.cpp
        IAudioMainWidow.h
        ClientManager.cpp
        ClientManager.h
        MonitorStream.cpp
        MonitorStream.h
        MainWidget.cpp
        MainWidget.h
        VersionStrategy.h
        ConcreteStrategy.cpp
        ConcreteStrategy.h
        TitleLabelItem.cpp
        TitleLabelItem.h
        Ukui4MainWidget.cpp
        Ukui4MainWidget.h
        ukui4.0/Ukui4SwitchButtonItem.cpp
        ukui4.0/Ukui4SwitchButtonItem.h
        ukui4.0/Ukui4VolumeSliderItem.cpp
        ukui4.0/Ukui4VolumeSliderItem.h
        ukui4.0/Ukui4SelectComboxItem.cpp
        ukui4.0/Ukui4SelectComboxItem.h
        ukui4.0/Audio.cpp
        ukui4.0/Audio.h
        Ukcc5DevicePortItem.cpp
        Ukcc5DevicePortItem.h
        Ukui5MainWidget.cpp
        Ukui5MainWidget.h
        Ukui5DetailSettingsItem.cpp
        Ukui5DetailSettingsItem.h
        IDeviceManagerItem.h
        DeviceManagerMainWidget.cpp
        DeviceManagerMainWidget.h
        DeviceManagerWidget.cpp
        DeviceManagerWidget.h
        AppManagerMainWidget.cpp
        AppManagerMainWidget.h
        AppManagerItemWidget.cpp
        AppManagerItemWidget.h
        AppManagerSelectComboxItem.cpp
        AppManagerSelectComboxItem.h
        AppManagerSliderItem.cpp
        AppManagerSliderItem.h
        ../common/Ukui4CustomControl.cpp
        ../common/Ukui4CustomControl.h
        ../common/DBusClient.cpp
        ../common/DBusClient.h
        ${QM_FILES}
        ${RESOURCES})
endif()

target_include_directories(audio_U5
    PUBLIC ../backend ../tray ../common
    )
target_include_directories(audio_testu2
    PUBLIC ../backend ../tray ../common
    )

target_link_libraries(audio_testu2
    ${KYSDK_LIBRARIES}
    ${GLIB2_LIBRARIES}
    ${PA_LIBRARIES}
    Qt5::Core
    Qt5::Widgets
    Qt5::DBus
    -lukcc)

target_link_libraries(audio_U5
    ${KYSDK_LIBRARIES}
    ${GLIB2_LIBRARIES}
    ${PA_LIBRARIES}
    ${QGSETTING_INCLUDE_DIRS}
    Qt5::Core
    Qt5::Widgets
    Qt5::DBus
    -lukcc
    PkgConfig::LibCanberra)

set(CMAKE_INSTALL_LIBDIR /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/ukui-control-center)

set(SOURCES main.cpp
    customstyle.cpp
    customstyle.h
    ClientMethod.cpp
    ClientMethod.h
    IDetailSettingsItem.cpp
    IDetailSettingsItem.h
    IVolumeSliderItem.cpp
    IVolumeSliderItem.h
    ISelectComboxItem.cpp
    ISelectComboxItem.h
    ISwitchButtonItem.h
    ISwitchButtonItem.cpp
    IAudioMainWidow.cpp
    IAudioMainWidow.h
    ClientManager.cpp
    ClientManager.h
    MonitorStream.cpp
    MonitorStream.h
    MainWidget.cpp
    MainWidget.h
    VersionStrategy.h
    ConcreteStrategy.h
    Ukui4MainWidget.cpp
    Ukui4MainWidget.h
    ukui4.0/Ukui4SwitchButtonItem.cpp
    ukui4.0/Ukui4SwitchButtonItem.h
    ukui4.0/Ukui4VolumeSliderItem.cpp
    ukui4.0/Ukui4VolumeSliderItem.h
    ukui4.0/Ukui4SelectComboxItem.cpp
    ukui4.0/Ukui4SelectComboxItem.h
    ukui4.0/Audio.cpp
    ukui4.0/Audio.h
    Ukcc5DevicePortItem.cpp
    Ukcc5DevicePortItem.h
    Ukui5MainWidget.cpp
    Ukui5MainWidget.h)

set(TRANSLATION_FILES translations/audio_zh_CN.ts)

add_custom_command(
    OUTPUT ${TRANSLATION_FILES}
    COMMAND ${Qt5_LUPDATE_EXECUTABLE}
    ARGS ${CMAKE_CURRENT_SOURCE_DIR}/src
    -ts ${TRANSLATION_FILES}
    DEPENDS ${SOURCES}
    COMMENT "Updating translations..."
    )

install(FILES ${TS_FILES} DESTINATION "/usr/share/ukui-media/translations/audio")
install(FILES ${QM_FILES} DESTINATION "/usr/share/ukui-media/translations/audio")

install(TARGETS ${PROJECT_NAME} DESTINATION "${CMAKE_INSTALL_LIBDIR}")

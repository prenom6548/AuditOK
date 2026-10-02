cmake_minimum_required(VERSION 3.16)

project(ukui-panel)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Quick Widgets DBus LinguistTools RemoteObjects REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Quick Widgets DBus LinguistTools RemoteObjects REQUIRED)
find_package(KF${QT_VERSION_MAJOR}WindowSystem REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(ukui-quick COMPONENTS core framework platform REQUIRED)

add_subdirectory(3rd-parties/qtsingleapplication)
include_directories(3rd-parties/qtsingleapplication/src)

include_directories(src)
include_directories(src/view)

file(GLOB TS_FILES translations/*.ts)
set(QRC_FILES qml/qml.qrc)

set(UKUI_PANEL_EXTERNAL_LIBS "")
if (QT_VERSION_MAJOR EQUAL "5")
    set(GSETTINGS_QT gsettings-qt)
elseif (QT_VERSION_MAJOR EQUAL "6")
    set(GSETTINGS_QT gsettings-qt6)
endif()

set(UKUI_PANEL_PC_PKGS ${GSETTINGS_QT} kysdk-waylandhelper kysdk-datacollect gio-2.0)

foreach(PC_LIB IN ITEMS ${UKUI_PANEL_PC_PKGS})
    pkg_check_modules(${PC_LIB} REQUIRED IMPORTED_TARGET ${PC_LIB})
    if(${${PC_LIB}_FOUND})
        include_directories(${${PC_LIB}_INCLUDE_DIRS})
        link_directories(${${PC_LIB}_LIBRARY_DIRS})
        list(APPEND UKUI_PANEL_EXTERNAL_LIBS PkgConfig::${PC_LIB})
    endif()
endforeach()

set(PROJECT_SOURCES
        src/main.cpp
        src/shell.cpp
        src/view/panel.cpp
        src/log-utils.cpp
        src/remote-config.cpp
        src/remote-config.h
        src/general-config-define.h
        src/screens-manager.cpp
        src/screens-manager.h
        src/panel-gsettings.cpp
        src/panel-gsettings.h
        src/view/widget-model.cpp
        src/view/widget-model.h
        src/view/common.h
        src/data-collector.h
        src/data-collector.cpp
        src/view/panel-next.cpp
        src/view/panel-next.h
        src/view/blur-region-helper.cpp
        src/view/blur-region-helper.h
        src/view/panel-top-bar.cpp
        src/view/panel-top-bar.h
        src/view/island-drag-helper.cpp
        src/view/island-drag-helper.h
        src/message-processor.cpp
        src/message-processor.h
)

if(COMMAND qt_add_dbus_adaptor)
    qt_add_dbus_adaptor(PROJECT_SOURCES dbus/org.ukui.panel.xml shell.h UkuiPanel::Shell)
else()
    qt5_add_dbus_adaptor(PROJECT_SOURCES dbus/org.ukui.panel.xml shell.h UkuiPanel::Shell)
endif()


if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/qml ${TS_FILES} OPTIONS -no-obsolete -no-ui-lines)
    qt_add_executable(${PROJECT_NAME}
            MANUAL_FINALIZATION
            ${PROJECT_SOURCES}
            ${QRC_FILES}
            ${QM_FILES}
    )
    qt6_add_repc_sources(${PROJECT_NAME} src/remote-config.rep)
    qt6_add_repc_replicas(${PROJECT_NAME} src/remote-config.rep)
else()
    qt5_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/qml ${TS_FILES} OPTIONS -no-obsolete -no-ui-lines)
    qt5_generate_repc(PROJECT_SOURCES src/remote-config.rep SOURCE)
    qt5_generate_repc(PROJECT_SOURCES src/remote-config.rep REPLICA)
    add_executable(${PROJECT_NAME}
            ${PROJECT_SOURCES}
            ${QRC_FILES}
            ${QM_FILES}
    )

endif()

target_include_directories(${PROJECT_NAME} BEFORE PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:PANEL_DISABLE_LOG_FILE>)
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:QT_QML_DEBUG>)
set(PANEL_TRANSLATION_DIR "/usr/share/ukui-panel/translations/")
add_compile_definitions(PANEL_TRANSLATION_DIR="${PANEL_TRANSLATION_DIR}")

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::RemoteObjects
        KF${QT_VERSION_MAJOR}::WindowSystem
        qtsingleapplication
        ${UKUI_PANEL_EXTERNAL_LIBS}
        ukui-quick::framework
        ukui-quick::core
        ukui-quick::platform
        ${GLIB_LIBRARIES}
)

add_subdirectory(ukcc-plugin)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "/usr/bin")
install(FILES ${QM_FILES} DESTINATION "${PANEL_TRANSLATION_DIR}")
install(FILES ukui-panel.desktop DESTINATION /etc/xdg/autostart)
install(FILES data/org.ukui.panel.settings.gschema.xml DESTINATION "/usr/share/glib-2.0/schemas")
install(FILES data/ukui-panel-v11.json DESTINATION "/opt/system/conf/ukui-security-support/")
if(BUILD_TEST)
    add_subdirectory(autotest)
endif()

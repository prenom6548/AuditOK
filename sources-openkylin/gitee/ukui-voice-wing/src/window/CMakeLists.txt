message(STATUS "Building ukui-voice-wing window")

set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Core Gui Widgets Qml Quick QuickControls2 REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(Gsettings REQUIRED IMPORTED_TARGET gsettings-qt)
pkg_check_modules(KysdkUkuiWindowHelper REQUIRED IMPORTED_TARGET kysdk-ukuiwindowhelper)
pkg_check_modules(WaylandClient REQUIRED IMPORTED_TARGET wayland-client)

set(WAYLAND_PROTOCOL_XML ${CMAKE_SOURCE_DIR}/proto/ukui-window-management.xml)
set(WAYLAND_PROTOCOL_DIR ${CMAKE_CURRENT_BINARY_DIR}/wayland-protocol)

add_custom_command(
    OUTPUT
        ${WAYLAND_PROTOCOL_DIR}/ukui-window-management-client-protocol.h
        ${WAYLAND_PROTOCOL_DIR}/ukui-window-management-protocol.c
    COMMAND ${CMAKE_COMMAND} -E make_directory ${WAYLAND_PROTOCOL_DIR}
    COMMAND wayland-scanner client-header ${WAYLAND_PROTOCOL_XML}
            ${WAYLAND_PROTOCOL_DIR}/ukui-window-management-client-protocol.h
    COMMAND wayland-scanner private-code ${WAYLAND_PROTOCOL_XML}
            ${WAYLAND_PROTOCOL_DIR}/ukui-window-management-protocol.c
    DEPENDS ${WAYLAND_PROTOCOL_XML}
    VERBATIM
)

add_library(${PROJECT_NAME}-window STATIC
    voiceInputWindow.cpp
    voiceInputWindow.h
    screenLocator.cpp
    screenLocator.h
    tipDialog.cpp
    tipDialog.h
    themeIconProvider.cpp
    themeIconProvider.h
    voiceInputWindow.qrc
    ${WAYLAND_PROTOCOL_DIR}/ukui-window-management-protocol.c
)

set_target_properties(${PROJECT_NAME}-window PROPERTIES
    AUTOMOC ON
    POSITION_INDEPENDENT_CODE ON
)

target_include_directories(${PROJECT_NAME}-window
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    PRIVATE
        ${WAYLAND_PROTOCOL_DIR}
)

set_source_files_properties(
    ${WAYLAND_PROTOCOL_DIR}/ukui-window-management-protocol.c
    PROPERTIES
        OBJECT_DEPENDS ${WAYLAND_PROTOCOL_DIR}/ukui-window-management-client-protocol.h
)

target_link_libraries(${PROJECT_NAME}-window
    PUBLIC
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Qml
        Qt5::Quick
        Qt5::QuickControls2
        PkgConfig::Gsettings
        PkgConfig::KysdkUkuiWindowHelper
        PkgConfig::WaylandClient
        ${PROJECT_NAME}-log
)

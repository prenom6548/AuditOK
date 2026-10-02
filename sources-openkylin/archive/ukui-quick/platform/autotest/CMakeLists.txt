set(CMAKE_AUTOMOC on)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Quick Test DBus Qml Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Quick Test DBus Qml Widgets REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(OpenGL REQUIRED)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# 查找 KWayland 库
find_package(KF5Wayland QUIET)

if(NOT DEFINED TEST_WITH_WAYLAND_COMPOSITOR)
    set(TEST_WITH_WAYLAND_COMPOSITOR 0)
endif()

add_executable(test-shadow-data
    ../ukui/shadow-data.cpp
    ../ukui/shadow-data.h
    test-shadow-data.cpp
)

target_include_directories(test-shadow-data PRIVATE ../ukui)

target_link_libraries(test-shadow-data PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Test
)

add_test(NAME test-shadow-data COMMAND test-shadow-data)

add_executable(test-gradient-color
    ../ukui/gradient-color.cpp
    ../ukui/gradient-color.h
    test-gradient-color.cpp
)

target_include_directories(test-gradient-color PRIVATE ../ukui)

target_link_libraries(test-gradient-color PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Test
)

add_test(NAME test-gradient-color COMMAND test-gradient-color)

add_executable(test-lottie-cache-manager
    test-lottie-cache-manager.cpp
    lottie-cache-manager-test.qrc
)

set_target_properties(test-lottie-cache-manager PROPERTIES AUTORCC ON)

target_include_directories(test-lottie-cache-manager PRIVATE
    ..
    ../ukui
)

target_link_libraries(test-lottie-cache-manager PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Test
    ukui-quick-lottie-core-test
)
target_compile_definitions(test-lottie-cache-manager PRIVATE
    UKUI_QUICK_LOTTIE_CORE_TEST_EXPORTS
)

add_test(NAME test-lottie-cache-manager COMMAND test-lottie-cache-manager)
set_tests_properties(test-lottie-cache-manager PROPERTIES
    TIMEOUT 15
)

add_executable(test-lottie-asset
    test-lottie-asset.cpp
)

target_include_directories(test-lottie-asset PRIVATE
    ..
    ../ukui
)

target_link_libraries(test-lottie-asset PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
    ukui-quick-lottie-core-test
)
target_compile_definitions(test-lottie-asset PRIVATE
    UKUI_QUICK_LOTTIE_CORE_TEST_EXPORTS
)

add_test(NAME test-lottie-asset COMMAND test-lottie-asset)

add_executable(test-dot-lottie-parser
    test-dot-lottie-parser.cpp
)

target_include_directories(test-dot-lottie-parser PRIVATE
    ..
    ../ukui
)

target_link_libraries(test-dot-lottie-parser PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
    ukui-quick-lottie-core-test
)
target_compile_definitions(test-dot-lottie-parser PRIVATE
    UKUI_QUICK_LOTTIE_CORE_TEST_EXPORTS
)

add_test(NAME test-dot-lottie-parser COMMAND test-dot-lottie-parser)

add_executable(test-application-icon-proxy
    test-application-icon-proxy.cpp
)

target_include_directories(test-application-icon-proxy PRIVATE
    ..
    ../ukui
)

target_link_libraries(test-application-icon-proxy PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
    ukui-quick::platform
)

add_test(NAME test-application-icon-proxy COMMAND test-application-icon-proxy)

add_executable(test-app-launcher
    test-app-launcher.cpp
)

target_include_directories(test-app-launcher PRIVATE
    ..
    ../ukui
)

target_link_libraries(test-app-launcher PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Test
    ukui-quick::platform
)

add_test(NAME test-app-launcher COMMAND test-app-launcher)

add_executable(test-wm-interface
    test-wm-interface.cpp
)

target_include_directories(test-wm-interface PRIVATE ..)

target_link_libraries(test-wm-interface PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Test
    ukui-quick::platform
)

add_test(NAME test-wm-interface COMMAND test-wm-interface)

add_executable(test-window-manager
    test-window-manager.cpp
)

target_include_directories(test-window-manager PRIVATE ..)

target_link_libraries(test-window-manager PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Test
    ukui-quick::platform
)

add_test(NAME test-window-manager COMMAND test-window-manager)

add_executable(test-abstract-window-manager
    test-abstract-window-manager.cpp
)

target_include_directories(test-abstract-window-manager PRIVATE ..)

target_link_libraries(test-abstract-window-manager PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Test
    ukui-quick::platform
)

add_test(NAME test-abstract-window-manager COMMAND test-abstract-window-manager)

add_executable(test-ukui-quick-platform-plugin
    test-ukui-quick-platform-plugin.cpp
)

target_include_directories(test-ukui-quick-platform-plugin PRIVATE ..)

target_link_libraries(test-ukui-quick-platform-plugin PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Test
    ukui-quick-platform-plugin
)

add_test(NAME test-ukui-quick-platform-plugin COMMAND test-ukui-quick-platform-plugin)

add_executable(test-region
    test-region.cpp
)

target_include_directories(test-region PRIVATE
    ..
    ../windows
)

target_link_libraries(test-region PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Test
    ukui-quick::platform
)

add_test(NAME test-region COMMAND test-region)

add_executable(test-screen-area-utils
    test-screen-area-utils.cpp
)

target_include_directories(test-screen-area-utils PRIVATE
    ..
    ../ukui
)

target_link_libraries(test-screen-area-utils PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Test
    ukui-quick::platform
)

add_test(NAME test-screen-area-utils COMMAND test-screen-area-utils)

add_executable(test-function-control
    test-function-control.cpp
)

target_include_directories(test-function-control PRIVATE
    ..
    ../ukui
)

target_link_libraries(test-function-control PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Test
    ukui-quick::platform
)

add_test(NAME test-function-control COMMAND test-function-control)

add_executable(test-dbus-connector
    test-dbus-connector.cpp
)

target_include_directories(test-dbus-connector PRIVATE
    ..
    ../ukui
)

target_link_libraries(test-dbus-connector PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Test
    ukui-quick::platform
)

add_test(NAME test-dbus-connector COMMAND test-dbus-connector)

add_executable(test-xatom-helper
    test-xatom-helper.cpp
)

target_include_directories(test-xatom-helper PRIVATE
    ..
)

target_link_libraries(test-xatom-helper PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Test
    ukui-quick::platform
)

add_test(NAME test-xatom-helper COMMAND test-xatom-helper)

add_executable(settings
    ../ukui/settings.cpp
    ../glinfo-query.cpp
    test-settings.cpp)
include_directories(../ukui)
target_link_libraries(settings PRIVATE Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Gui
    OpenGL::GL
    X11
    EGL
    wayland-client)
add_test(NAME settings COMMAND settings)

add_executable(test-theme
    ../ukui/ukui-theme-proxy.cpp
    test-theme.cpp)
include_directories(../ukui)

pkg_check_modules(gsettings-qt REQUIRED IMPORTED_TARGET gsettings-qt)
if(${gsettings-qt_FOUND})
    include_directories(${gsettings-qt_INCLUDE_DIRS})
    link_directories(${gsettings-qt_LIBRARY_DIRS})
    list(APPEND EXTERNAL_LIBS PkgConfig::gsettings-qt)
endif()

target_link_libraries(test-theme PRIVATE Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Widgets
    gsettings-qt)
add_test(NAME test-theme COMMAND test-theme)

set(WAYLAND_PROTOCOLS_SRCS
        ../wayland/registry.cpp
        ../wayland/wayland-pointer_p.h
        ../wayland/wayland-integration.cpp
        ../wayland/ukui-shell-v1.h
        ../wayland/ukui-shell-v1.cpp
        ../wayland/ukui-window-management.h
        ../wayland/ukui-window-management.cpp
        ../wayland/ukui-startup-manager.cpp
        ../wayland/ukui-startup-manager.h
        ../wayland/ukui-blur-manager.cpp
        ../wayland/ukui-blur-manager.h
        ../wayland/compositor.cpp
        ../wayland/compositor.h
        ../wayland/ukui-workspace-manager.cpp
        ../wayland/ukui-workspace-manager.h
        ../wayland/kywc-output-manager.cpp
        ../wayland/kywc-output-manager.h
        ../wayland/kywc-capture-manager.cpp
        ../wayland/kywc-capture-manager.h
 )
ukui_add_wayland_client_protocol(WAYLAND_PROTOCOLS_SRCS
        PROTOCOL ../wayland/protocol/ukui-shell-v1.xml
        BASENAME ukui-shell
        PRIVATE_CODE
)

ukui_add_wayland_client_protocol(WAYLAND_PROTOCOLS_SRCS
        PROTOCOL ../wayland/protocol/ukui-window-management.xml
        BASENAME ukui-window-management
        PRIVATE_CODE
)

ukui_add_wayland_client_protocol(WAYLAND_PROTOCOLS_SRCS
        PROTOCOL ${PLASMA_WAYLAND_PROTOCOLS_DIR}/plasma-virtual-desktop.xml
        BASENAME plasma-virtual-desktop
        PRIVATE_CODE
)

ukui_add_wayland_client_protocol(WAYLAND_PROTOCOLS_SRCS
        PROTOCOL ../wayland/protocol//ukui-startup-v1.xml
        BASENAME ukui-startup-v1
        PRIVATE_CODE
)

ukui_add_wayland_client_protocol(WAYLAND_PROTOCOLS_SRCS
        PROTOCOL ../wayland/protocol//ukui-blur-v1.xml
        BASENAME ukui-blur-v1
        PRIVATE_CODE
)

ukui_add_wayland_client_protocol(WAYLAND_PROTOCOLS_SRCS
        PROTOCOL ../wayland/protocol/kywc-workspace-v1.xml
        BASENAME kywc-workspace-v1
        PRIVATE_CODE
)

ukui_add_wayland_client_protocol(WAYLAND_PROTOCOLS_SRCS
        PROTOCOL ../wayland/protocol/kywc-output-v1.xml
        BASENAME kywc-output-v1
        PRIVATE_CODE
)

ukui_add_wayland_client_protocol(WAYLAND_PROTOCOLS_SRCS
        PROTOCOL ../wayland/protocol/kywc-capture-v1.xml
        BASENAME kywc-capture-v1
        PRIVATE_CODE
)

set(TEST_KYWC_OUTPUT_MANAGER_PROTOCOL_SRCS)
ukui_add_wayland_client_protocol(TEST_KYWC_OUTPUT_MANAGER_PROTOCOL_SRCS
        PROTOCOL ../wayland/protocol/kywc-output-v1.xml
        BASENAME kywc-output-v1
        PRIVATE_CODE
)

# 添加头文件搜索路径
include_directories(
        ../
        ../wayland
        ${CMAKE_SOURCE_DIR}
)

# 查找 Wayland 客户端库
pkg_check_modules(wayland-client REQUIRED IMPORTED_TARGET wayland-client)
if(${wayland-client_FOUND})
    include_directories(${wayland-client_INCLUDE_DIRS})
    link_directories(${wayland-client_LIBRARY_DIRS})
endif()

add_executable(test-kywc-output-manager
        test-kywc-output-manager.cpp
        ../wayland/kywc-output-manager.h
        ${TEST_KYWC_OUTPUT_MANAGER_PROTOCOL_SRCS}
)

target_link_libraries(test-kywc-output-manager PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Core
        wayland-client
        Qt${QT_VERSION_MAJOR}::GuiPrivate
        KF5::WaylandClient
)

add_test(NAME test-kywc-output-manager COMMAND test-kywc-output-manager)

set(TEST_PUBLIC_OUTPUT_MANAGER_SRCS ${WAYLAND_PROTOCOLS_SRCS})
list(REMOVE_ITEM TEST_PUBLIC_OUTPUT_MANAGER_SRCS ../wayland/kywc-output-manager.cpp)

add_executable(test-public-output-manager
        test-public-output-manager.cpp
        ../output-manager.h
        ${TEST_PUBLIC_OUTPUT_MANAGER_SRCS}
)

target_link_libraries(test-public-output-manager PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Core
        wayland-client
        Qt${QT_VERSION_MAJOR}::GuiPrivate
        KF5::WaylandClient
)

add_test(NAME test-public-output-manager COMMAND test-public-output-manager)

# test workspace manager
set(TEST_WORKSPACE_MANAGER_SRCS
        test-workspace-manager.cpp
)

add_executable(test-workspace-manager
        ${TEST_WORKSPACE_MANAGER_SRCS}
        ${WAYLAND_PROTOCOLS_SRCS}
)

include_directories(
        ../
        ../wayland
        ${CMAKE_SOURCE_DIR}
)

target_compile_definitions(test-workspace-manager PRIVATE
        TEST_WITH_WAYLAND_COMPOSITOR=${TEST_WITH_WAYLAND_COMPOSITOR}
        UKUI_WORKSPACE_MANAGER_ENABLE_TEST_HOOKS
)

target_link_libraries(test-workspace-manager PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Core
        wayland-client
        Qt${QT_VERSION_MAJOR}::GuiPrivate
        KF5::WaylandClient
)

add_test(NAME test-workspace-manager COMMAND test-workspace-manager)

set_tests_properties(test-workspace-manager PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

add_executable(test-heartbeat-protocol
        test-heartbeat-protocol.cpp
        ../ukui/heartbeat/protocol.cpp
)

target_link_libraries(test-heartbeat-protocol PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
)

add_test(NAME test-heartbeat-protocol COMMAND test-heartbeat-protocol)

add_executable(test-heartbeat-client
        test-heartbeat-client.cpp
        ../ukui/heartbeat/keep-alive-client.h
        ../ukui/heartbeat/protocol.cpp
)

target_link_libraries(test-heartbeat-client PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
)

add_test(NAME test-heartbeat-client COMMAND test-heartbeat-client)

add_executable(test-public-workspace-manager
        test-public-workspace-manager.cpp
)

target_include_directories(test-public-workspace-manager PRIVATE
        ..
        ../wayland
)

target_link_libraries(test-public-workspace-manager PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        ukui-quick::platform
)

add_test(NAME test-public-workspace-manager COMMAND test-public-workspace-manager)

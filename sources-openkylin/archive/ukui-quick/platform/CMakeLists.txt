cmake_minimum_required(VERSION 3.16)
project(platform)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)

include(FeatureSummary)
find_package(ECM NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)
list(PREPEND CMAKE_MODULE_PATH
        ${ECM_MODULE_PATH}
        ${CMAKE_SOURCE_DIR}/cmake
)


find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Quick Qml Widgets DBus Concurrent WaylandClient  REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Quick Qml Widgets DBus Concurrent WaylandClient REQUIRED)

if (QT_VERSION_MAJOR EQUAL "5")
    set(KF_VERSION_MAJOR "5")
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS X11Extras XkbCommonSupport REQUIRED)
    find_package(KF5WindowSystem REQUIRED)
    find_package(KF5Wayland)
    set(PC_PKGS gsettings-qt x11 Qt5Xdg wayland-client)
elseif (QT_VERSION_MAJOR EQUAL "6")
    set(KF_VERSION_MAJOR "6")
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Gui REQUIRED)
    find_package(KF6WindowSystem REQUIRED)
    find_package(KWayland)
    set(PC_PKGS gsettings-qt6 x11 Qt6Xdg wayland-client)
endif()

find_package(PkgConfig REQUIRED)
find_package(WaylandScanner)
find_package(WaylandProtocols 1.15)
set_package_properties(WaylandProtocols PROPERTIES TYPE REQUIRED)
find_package(PlasmaWaylandProtocols 1.4.0 CONFIG)
set_package_properties(PlasmaWaylandProtocols PROPERTIES TYPE REQUIRED)
include(./cmake/FindWaylandScanner.cmake)

set(EXTERNAL_LIBS "")

    foreach(external_lib IN ITEMS ${PC_PKGS})
    pkg_check_modules(${external_lib} REQUIRED IMPORTED_TARGET ${external_lib})
    if(${${external_lib}_FOUND})
        include_directories(${${external_lib}_INCLUDE_DIRS})
        link_directories(${${external_lib}_LIBRARY_DIRS})
        list(APPEND EXTERNAL_LIBS PkgConfig::${external_lib})
    endif()
endforeach()

set(UKUI_QUICK_PLATFORM_SRCS
        ukui/settings.cpp
        ukui/app-launcher.cpp
        ukui/ukui-theme-proxy.cpp
        ukui/screen-area-utils.cpp
        ukui/dt-theme.cpp ukui/dt-theme.h
        ukui/dt-theme-definition.h
        ukui/dt-theme-definition.cpp
        ukui/gradient-color.h
        ukui/gradient-color.cpp
        ukui/shadow-data.h
        ukui/shadow-data.cpp
        ukui/dbus-connector.cpp
        ukui/function-control.cpp
        wayland/registry.cpp
        wayland/wayland-pointer_p.h
        wayland/wayland-integration.cpp
        wayland/ukui-shell-v1.h
        wayland/ukui-shell-v1.cpp
        wayland/ukui-window-management.h
        wayland/ukui-window-management.cpp
        wayland/ukui-startup-manager.cpp
        wayland/ukui-startup-manager.h
        wayland/ukui-blur-manager.cpp
        wayland/ukui-blur-manager.h
        wayland/compositor.cpp
        wayland/compositor.h
        wayland/ukui-workspace-manager.cpp
        wayland/ukui-workspace-manager.h
        wayland/kywc-output-manager.cpp
        wayland/kywc-output-manager.h
        wayland/kywc-capture-manager.cpp
        wayland/kywc-capture-manager.h
        windows/dialog.cpp
        windows/region.cpp
        windows/ukui-window.cpp
        window-helper.cpp
        wm-interface.cpp
        wm-impl-x11.h wm-impl-x11.cpp
        wm-impl-wayland.cpp wm-impl-wayland.h
        abstract-window-manager.cpp
        wayland-window-manager.cpp wayland-window-manager.h
        window-manager.cpp
        workspace-manager.cpp
        xcb-window-manager.cpp xcb-window-manager.h
        xatom-helper.cpp
        startup-management.cpp
        ukui/application-icon-proxy.cpp
        ukui/application-icon-proxy.h
        glinfo-query.cpp
        glinfo-query.h
        ukui/heartbeat/keep-alive-client.cpp
        ukui/heartbeat/keep-alive-client.h
        ukui/heartbeat/protocol.cpp
        ukui/heartbeat/protocol.h
        output-manager.cpp
)
set(HEADERS
        ukui/settings.h
        ukui/app-launcher.h
        ukui/ukui-theme-proxy.h
        ukui/screen-area-utils.h
        ukui/dbus-connector.h
        ukui/function-control.h
        window-type.h
        wm-interface.h
        window-helper.h
        wayland/registry.h
        windows/dialog.h
        windows/ukui-window.h
        windows/region.h
        window-manager.h
        workspace-manager.h
        abstract-window-manager.h
        startup-management.h
        ukui/heartbeat/keep-alive-client.h
        output-manager.h
)
include_directories(wayland)
ukui_add_wayland_client_protocol(UKUI_QUICK_PLATFORM_SRCS
        PROTOCOL wayland/protocol/ukui-shell-v1.xml
        BASENAME ukui-shell
        PRIVATE_CODE
)

ukui_add_wayland_client_protocol(UKUI_QUICK_PLATFORM_SRCS
        PROTOCOL wayland/protocol/ukui-window-management.xml
        BASENAME ukui-window-management
        PRIVATE_CODE
)

ukui_add_wayland_client_protocol(UKUI_QUICK_PLATFORM_SRCS
        PROTOCOL ${PLASMA_WAYLAND_PROTOCOLS_DIR}/plasma-virtual-desktop.xml
        BASENAME plasma-virtual-desktop
        PRIVATE_CODE
)

ukui_add_wayland_client_protocol(UKUI_QUICK_PLATFORM_SRCS
        PROTOCOL wayland/protocol//ukui-startup-v1.xml
        BASENAME ukui-startup-v1
        PRIVATE_CODE
)

ukui_add_wayland_client_protocol(UKUI_QUICK_PLATFORM_SRCS
        PROTOCOL wayland/protocol//ukui-blur-v1.xml
        BASENAME ukui-blur-v1
        PRIVATE_CODE
)

ukui_add_wayland_client_protocol(UKUI_QUICK_PLATFORM_SRCS
        PROTOCOL wayland/protocol/kywc-workspace-v1.xml
        BASENAME kywc-workspace-v1
        PRIVATE_CODE
)

ukui_add_wayland_client_protocol(UKUI_QUICK_PLATFORM_SRCS
        PROTOCOL wayland/protocol/kywc-output-v1.xml
        BASENAME kywc-output-v1
        PRIVATE_CODE
)

ukui_add_wayland_client_protocol(UKUI_QUICK_PLATFORM_SRCS
        PROTOCOL wayland/protocol/kywc-capture-v1.xml
        BASENAME kywc-capture-v1
        PRIVATE_CODE
)

add_library(${PROJECT_NAME} SHARED
        ${UKUI_QUICK_PLATFORM_SRCS} ${HEADERS}
)
add_library(${ROOT_PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Concurrent
        Qt${QT_VERSION_MAJOR}::GuiPrivate
        Qt${QT_VERSION_MAJOR}::WaylandClientPrivate
        KF${KF_VERSION_MAJOR}::WindowSystem
        ${EXTERNAL_LIBS}
)
if (QT_VERSION_MAJOR EQUAL "5")
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt::X11Extras Qt5::XkbCommonSupportPrivate Qt5::Gui_EGL KF5::WaylandClient)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt::GuiPrivate Plasma::KWaylandClient)
endif()

set(HEADERS_INSTALL_DIR /usr/include/ukui-quick/platform)
target_include_directories(${PROJECT_NAME} INTERFACE $<INSTALL_INTERFACE:${HEADERS_INSTALL_DIR}>)

include(CMakePackageConfigHelpers)
set(CMAKE_CONFIG_INSTALL_DIR "/usr/share/cmake/ukui-quick-platform")
set(PC_INSTALL_DIR "/usr/lib/pkgconfig")

configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/ukui-quick-platform-config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/ukui-quick-platform-config.cmake"
        INSTALL_DESTINATION  ${CMAKE_CONFIG_INSTALL_DIR}
)

write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/ukui-quick-platform-config-version.cmake
        VERSION ${UKUI_QUICK_VERSION}
        COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/ukui-quick-platform.pc.in"
        "${CMAKE_CURRENT_BINARY_DIR}/ukui-quick-platform.pc"
        INSTALL_DESTINATION  ${PC_INSTALL_DIR})

set_target_properties(${PROJECT_NAME} PROPERTIES
        VERSION ${UKUI_QUICK_VERSION}
        SOVERSION ${VERSION_MAJOR}
        OUTPUT_NAME ${ROOT_PROJECT_NAME}-${PROJECT_NAME}
)
install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}
        PUBLIC_HEADER DESTINATION ${HEADERS_INSTALL_DIR}
        LIBRARY DESTINATION /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}
)
install(EXPORT ${PROJECT_NAME}
        FILE ukui-quick-platform-targets.cmake
        NAMESPACE ${ROOT_PROJECT_NAME}::
        DESTINATION ${CMAKE_CONFIG_INSTALL_DIR})

install(FILES ${HEADERS} DESTINATION ${HEADERS_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ukui-quick-platform.pc DESTINATION ${PC_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ukui-quick-platform-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/ukui-quick-platform-config-version.cmake
        DESTINATION ${CMAKE_CONFIG_INSTALL_DIR})

set(UKUI_QUICK_PLATFORM_PLUGIN_SRCS
        ukui-quick-platform-plugin.cpp
        ukui-quick-platform-plugin.h
)
add_library(${ROOT_PROJECT_NAME}-${PROJECT_NAME}-plugin SHARED ${UKUI_QUICK_PLATFORM_PLUGIN_SRCS})

target_compile_definitions(${ROOT_PROJECT_NAME}-${PROJECT_NAME}-plugin PRIVATE
        PLUGIN_IMPORT_URI="org.ukui.quick.platform"
        PLUGIN_VERSION_MAJOR=${VERSION_MAJOR}
        PLUGIN_VERSION_MINOR=${VERSION_MINOR}
)
target_link_libraries(${ROOT_PROJECT_NAME}-${PROJECT_NAME}-plugin
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Quick
        ${PROJECT_NAME}
)
set(PLUGIN_INSTALL_ROOT_PATH "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/qt${QT_VERSION_MAJOR}/qml/org/ukui/quick/platform")
install(FILES qmldir DESTINATION "${PLUGIN_INSTALL_ROOT_PATH}")
install(TARGETS ${ROOT_PROJECT_NAME}-${PROJECT_NAME}-plugin LIBRARY DESTINATION "${PLUGIN_INSTALL_ROOT_PATH}")
install(FILES ukui/DtThemeDefault.qml DESTINATION "/usr/share/ukui/ukui-quick-platform")



# 文档生成配置
include(UkuiQDocAddQch)
set(include_dirs
        "/usr/include/${CMAKE_LIBRARY_ARCHITECTURE}/qt${QT_VERSION_MAJOR}/"
        "/usr/include/${CMAKE_LIBRARY_ARCHITECTURE}/qt${QT_VERSION_MAJOR}/QtCore"
        "/usr/include/${CMAKE_LIBRARY_ARCHITECTURE}/qt${QT_VERSION_MAJOR}/QtQml"
        "/usr/include/${CMAKE_LIBRARY_ARCHITECTURE}/qt${QT_VERSION_MAJOR}/QtQuick"
        "/usr/include/${CMAKE_LIBRARY_ARCHITECTURE}/qt${QT_VERSION_MAJOR}/QtWidgets"
        "/usr/include/${CMAKE_LIBRARY_ARCHITECTURE}/qt${QT_VERSION_MAJOR}/QtGui"
        "/usr/include/${CMAKE_LIBRARY_ARCHITECTURE}/qt${QT_VERSION_MAJOR}/QtDBus"
        "/usr/include/${CMAKE_LIBRARY_ARCHITECTURE}/qt${QT_VERSION_MAJOR}/QtConcurrent"
        "/usr/include/${CMAKE_LIBRARY_ARCHITECTURE}/qt${QT_VERSION_MAJOR}/QtWaylandClient"
)
ukui_qdoc_add_qch(ukui-quick-platform-doc
        NAME ukui-quick-platform
        DOC_NAME ukui-quick-platform
        VERSION ${UKUI_QUICK_VERSION}
        NAMESPACE org.ukui
        QCH_INSTALL_DESTINATION "/usr/share/qt${QT_VERSION_MAJOR}/doc"
        SOURCE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
        HEADER_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
        EXCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/ukui/lottie
        CONFIG_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/../cmake-extend/modules/QDocConfig.qdocconf.in"
        INCLUDE_DIRS ${include_dirs}
        DEPEND_QCHS qtcore qtqml qtquick
        STYLE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../cmake-extend/style/default.css"
)

enable_testing()
if(BUILD_TEST)
    add_subdirectory(autotest)
endif ()
if(BUILD_TEST AND BUILD_INTERACTIVE_TESTS)
    add_subdirectory(test)
endif ()

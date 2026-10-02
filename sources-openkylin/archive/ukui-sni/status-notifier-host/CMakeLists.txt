project(ukui-sni-host VERSION 1.0 LANGUAGES C CXX)
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_MICRO 0)
set(UKUI_SNI_HOST_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO})
include_directories(../common)
set(ukui-sni-host_LIB_SRCS
        status-notifier-host.cpp
        ../common/dbus-types.cpp
        status-notifier-item.cpp
        status-notifier-item-private.h
        libdbusmenu-qt/dbusmenu-importer.cpp
        libdbusmenu-qt/dbusmenu-importer.h
        libdbusmenu-qt/dbusmenu-shortcut_p.cpp
        libdbusmenu-qt/dbusmenu-shortcut_p.h
        libdbusmenu-qt/dbusmenu-types_p.cpp
        libdbusmenu-qt/dbusmenu-types_p.h
        libdbusmenu-qt/utils.cpp
        libdbusmenu-qt/utils_p.h
)
set(HEADERS
        status-notifier-host.h
        status-notifier-item.h
        )
set(statusnotifierwatcher_xml ../dbus/org.kde.StatusNotifierWatcher.xml)
set_source_files_properties(${statusnotifierwatcher_xml} PROPERTIES
        NO_NAMESPACE false
        INCLUDE "../common/dbus-types.h"
)
qt_add_dbus_interface(ukui-sni-host_LIB_SRCS ${statusnotifierwatcher_xml} statusnotifierwatcher_interface)
qt_add_dbus_interface(ukui-sni-host_LIB_SRCS ../dbus/org.freedesktop.DBus.Properties.xml dbusproperties)
set(statusnotifieritem_xml ../dbus/org.kde.StatusNotifierItem.xml)
set_source_files_properties(${statusnotifieritem_xml} PROPERTIES
        NO_NAMESPACE false
        INCLUDE "../common/dbus-types.h"
        CLASSNAME OrgKdeStatusNotifierItem
        )
qt_add_dbus_interface(ukui-sni-host_LIB_SRCS ${statusnotifieritem_xml} statusnotifieritem_interface)

set_source_files_properties(libdbusmenu-qt/com.canonical.dbusmenu.xml PROPERTIES
        NO_NAMESPACE true
        INCLUDE "dbusmenu-types_p.h"
        CLASSNAME DBusMenuInterface
)
qt_add_dbus_interface(ukui-sni-host_LIB_SRCS libdbusmenu-qt/com.canonical.dbusmenu.xml dbusmenu_interface)
include_directories(../common libdbusmenu-qt)
add_library(${PROJECT_NAME} SHARED ${ukui-sni-host_LIB_SRCS})
set_target_properties(${PROJECT_NAME} PROPERTIES
        VERSION ${UKUI_SNI_HOST_VERSION}
        SOVERSION ${VERSION_MAJOR}
        OUTPUT_NAME ${PROJECT_NAME}
)
target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        )

if (QT_VERSION_MAJOR EQUAL 5)
        target_link_libraries(${PROJECT_NAME} PRIVATE
                Qt5::X11Extras
        )
elseif(QT_VERSION_MAJOR EQUAL 6)
        target_link_libraries(${PROJECT_NAME} PRIVATE
                Qt::GuiPrivate
        )
endif()

include(CMakePackageConfigHelpers)
set(CMAKE_CONFIG_INSTALL_DIR "/usr/share/cmake/ukui-sni-host")
set(HEADERS_INSTALL_DIR /usr/include/ukui-sni-host)
set(PC_INSTALL_DIR "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/pkgconfig")

target_include_directories(${PROJECT_NAME}  PUBLIC  $<INSTALL_INTERFACE:${HEADERS_INSTALL_DIR}>)

configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/ukui-sni-host-config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/ukui-sni-host-config.cmake"
        INSTALL_DESTINATION  ${CMAKE_CONFIG_INSTALL_DIR})
write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/ukui-sni-host-config-version.cmake
        VERSION ${LIBUKUI_SEARCH_VERSION}
        COMPATIBILITY SameMajorVersion
)
configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/ukui-sni-host.pc.in"
        "${CMAKE_CURRENT_BINARY_DIR}/pkgconfig/ukui-sni-host.pc"
        INSTALL_DESTINATION  ${PC_INSTALL_DIR})

install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}
        PUBLIC_HEADER DESTINATION ${HEADERS_INSTALL_DIR}
        LIBRARY DESTINATION /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}
)
install(EXPORT ${PROJECT_NAME}
        FILE ${PROJECT_NAME}-targets.cmake
        DESTINATION ${CMAKE_CONFIG_INSTALL_DIR})
install(FILES ${HEADERS} DESTINATION ${HEADERS_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pkgconfig/ukui-sni-host.pc DESTINATION ${PC_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ukui-sni-host-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/ukui-sni-host-config-version.cmake
        DESTINATION ${CMAKE_CONFIG_INSTALL_DIR})
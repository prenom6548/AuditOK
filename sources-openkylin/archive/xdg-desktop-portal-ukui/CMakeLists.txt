cmake_minimum_required(VERSION 3.5)
project(xdg-desktop-portal-ukui VERSION 1.0 LANGUAGES C CXX)

include(GNUInstallDirs)
include_directories(src)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INSTALL_PREFIX "/usr")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Widgets Core Gui DBus LinguistTools PrintSupport REQUIRED)
find_package(KF5WindowSystem REQUIRED)
find_package(KF5Wayland REQUIRED)
find_package(KF5CoreAddons REQUIRED)
find_package(PkgConfig)

set(XDP_UKUI_PC_PKGS gio-2.0 peony kysdk-waylandhelper kysdk-qtwidgets ukui-search kysdk-ukuiwindowhelper)
foreach(external_lib IN ITEMS ${XDP_UKUI_PC_PKGS})
    pkg_check_modules(${external_lib} REQUIRED IMPORTED_TARGET ${external_lib})
    if(${${external_lib}_FOUND})
        include_directories(${${external_lib}_INCLUDE_DIRS})
        link_directories(${${external_lib}_LIBRARY_DIRS})
        list(APPEND XDP_UKUI_EXTERNAL_LIBS PkgConfig::${external_lib})
    endif()
endforeach()


set(LIBEXEC_PATH ${CMAKE_INSTALL_FULL_LIBEXECDIR})
configure_file(data/org.freedesktop.impl.portal.desktop.ukui.desktop.in ${CMAKE_CURRENT_SOURCE_DIR}/data/org.freedesktop.impl.portal.desktop.ukui.desktop)
configure_file(data/org.freedesktop.impl.portal.desktop.ukui.service.in ${CMAKE_CURRENT_SOURCE_DIR}/data/org.freedesktop.impl.portal.desktop.ukui.service)
set(xdg-desktop-portal-ukui_SRCS
        src/filechooser/debug.h
        src/filechooser/kyfiledialog.cpp src/filechooser/kyfiledialog.h
        src/filechooser/kyfiledialogprivate.cpp src/filechooser/kyfiledialogprivate.h
        src/filechooser/menutoolbutoon.cpp src/filechooser/menutoolbutoon.h
        src/filechooser/pathbar.cpp src/filechooser/pathbar.h
        src/filechooser/sidebar.cpp src/filechooser/sidebar.h
        src/filechooser/uikyfiledialog.cpp src/filechooser/uikyfiledialog.h
        src/filechooser/file-operation-dialog/kyfiledialogrename.cpp src/filechooser/file-operation-dialog/kyfiledialogrename.h
        src/filechooser/file-operation-dialog/rename-editor.cpp src/filechooser/file-operation-dialog/rename-editor.h
        src/filechooser/file-operation-dialog/kyfileoperationdialog.cpp src/filechooser/file-operation-dialog/kyfileoperationdialog.h
        src/request.cpp src/request.h
        src/ukuidesktopportal.cpp src/ukuidesktopportal.h
        src/ukuifilechooserportal.cpp src/ukuifilechooserportal.h
        src/ukuisessioninhibitportal.cpp src/ukuisessioninhibitportal.h
        src/utils/windowutils.cpp src/utils/windowutils.h
        src/xdg-desktop-portal-ukui.cpp
        src/notification/notification-portal.cpp
        src/notification/notification-portal.h
        src/notification/portal-icon.cpp
        src/notification/portal-icon.h
        src/appchooser/appchooser.h src/appchooser/appchooser.cpp
        src/utils/waylandintegration.h src/utils/waylandintegration.cpp
        src/print.h src/print.cpp
)
set_source_files_properties(
        src/notification/org.freedesktop.Notifications.xml
        PROPERTIES
        CLASSNAME
        NotificationService
        NO_NAMESPACE
        OFF
)
if(COMMAND qt_add_dbus_interface)
    qt_add_dbus_interface(xdg-desktop-portal-ukui_SRCS src/notification/org.freedesktop.Notifications.xml notifications_interface)
else()
    qt5_add_dbus_interface(xdg-desktop-portal-ukui_SRCS src/notification/org.freedesktop.Notifications.xml notifications_interface)
endif()

add_executable(xdg-desktop-portal-ukui ${xdg-desktop-portal-ukui_SRCS})
target_include_directories(xdg-desktop-portal-ukui PRIVATE
        src/filechooser
        src/filechooser/../utils
        src/notification
        ${GIO_INCLUDE_DIRS}
        ${PEONY_INCLUDE_DIRS}
        ${KYSDK_WAYLANDHELPER_INCLUDE_DIRS}
        ${KYSDK_QTWIDGETS_INCLUDE_DIRS}
)

target_link_libraries(xdg-desktop-portal-ukui PRIVATE
        KF5::WindowSystem
        KF5::WaylandClient
        KF5::CoreAddons
        Qt5::Core
        Qt5::DBus
        Qt5::Gui
        Qt5::Widgets
        Qt5::PrintSupport
        ${XDP_UKUI_EXTERNAL_LIBS}
        ${GIO_LIBRARIES}
        ${PEONY_LIBRARIES}
        ${KYSDK_WAYLANDHELPER_LIBRARIES}
        ${KYSDK_QTWIDGETS_LIBRARIES}
        -lukui-log4qt
)

target_link_libraries(xdg-desktop-portal-ukui PRIVATE
        Qt5::PrintSupportPrivate
        Qt5::GuiPrivate
)

# i18n
set(UKUI_TRANSLATIONS_DIR "/usr/share/ukui/xdg-desktop-portal-ukui")
add_definitions(
    -DUKUI_TRANSLATIONS_DIR="${UKUI_TRANSLATIONS_DIR}"
)
if (NOT DEFINED UPDATE_TRANSLATIONS)
    set(UPDATE_TRANSLATIONS "No")
endif()

# To create a new ts file: lupdate -recursive . -target-language en_US -ts translations/en_US.ts -no-ui-lines
file(GLOB TS_FILES "${CMAKE_CURRENT_SOURCE_DIR}/translations/*.ts")

# cmake -DUPDATE_TRANSLATIONS=yes
if (UPDATE_TRANSLATIONS)
	find_program(LUPDATE_BIN lupdate)
	foreach(_ts_file ${TS_FILES})
		execute_process(
			COMMAND ${LUPDATE_BIN} -recursive ${CMAKE_CURRENT_SOURCE_DIR} -ts ${_ts_file}
		)
	endforeach()
endif()

set(PLUGIN_INSTALL_FILES /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/peony-extensions/libpeony-computer-view-plugin.so)
target_compile_definitions(${PROJECT_NAME} PRIVATE PLUGIN_INSTALL_FILES="${PLUGIN_INSTALL_FILES}")

set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION ${CMAKE_SOURCE_DIR}/translations)
qt5_add_translation(QM_FILES ${TS_FILES})
add_custom_target(translations ALL DEPENDS ${QM_FILES})

install(TARGETS xdg-desktop-portal-ukui DESTINATION ${CMAKE_INSTALL_FULL_LIBEXECDIR})
install(FILES data/org.freedesktop.impl.portal.desktop.ukui.desktop DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/applications)
install(FILES data/org.freedesktop.impl.portal.desktop.ukui.service DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/dbus-1/services)
install(FILES data/50xdp-ukui DESTINATION ${CMAKE_INSTALL_FULL_SYSCONFDIR}/X11/Xsession.d)
install(FILES data/ukui.portal DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/xdg-desktop-portal/portals)
install(FILES ${TS_FILES} ${QM_FILES} DESTINATION ${UKUI_TRANSLATIONS_DIR})

#if(NOT "$ENV{XDG_UKUI_SKIP_TEST}" STREQUAL "1")
#    add_subdirectory(test)
#    enable_testing()
#endif()

set(XDG_UKUI_SKIP_TEST_VALUE $ENV{XDG_UKUI_SKIP_TEST})
# 判断是否跳过测试目录的编译
if(NOT XDG_UKUI_SKIP_TEST_VALUE STREQUAL "1")
    enable_testing()
    add_subdirectory(test)
else()
    message(STATUS "Skipping compilation of test directory due to XDG_UKUI_SKIP_TEST=1")
endif()


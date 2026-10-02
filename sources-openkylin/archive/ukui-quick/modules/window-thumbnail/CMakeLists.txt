cmake_minimum_required(VERSION 3.16)

project(ukui-window-thumbnail LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#find QT modules
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Qml Quick DBus REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Qml Gui Quick DBus REQUIRED)

#find kde modules
#find_package(Qt5WaylandClient)

if (QT_VERSION_MAJOR EQUAL "5")
    set(KF_VERSION_MAJOR "5")
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS X11Extras REQUIRED)
    find_package(KF5WindowSystem REQUIRED)
    find_package(KF5Wayland)
    find_package(PlasmaWaylandProtocols 1.6)
elseif (QT_VERSION_MAJOR EQUAL "6")
    set(KF_VERSION_MAJOR "6")
    find_package(KF6WindowSystem REQUIRED)
    find_package(KWayland)
    find_package(PlasmaWaylandProtocols 1.16)
endif()

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
include(FindQtWaylandScanner)

#find other modules
find_package(PkgConfig REQUIRED)
set(WINDOW_THUMBNAIL_PLUGIN_PC_PKGS libpipewire-0.3 x11 xcb xcb-damage xcb-composite egl xcb-render wayland-client glx)
foreach(external_lib IN ITEMS ${WINDOW_THUMBNAIL_PLUGIN_PC_PKGS})
    pkg_check_modules(${external_lib} REQUIRED IMPORTED_TARGET ${external_lib})
    if(${${external_lib}_FOUND})
        include_directories(${${external_lib}_INCLUDE_DIRS})
        link_directories(${${external_lib}_LIBRARY_DIRS})
        list(APPEND WINDOW_THUMBNAIL_PLUGIN_EXTERNAL_LIBS PkgConfig::${external_lib})
    endif()
endforeach()

include_directories(mpris)
set(QRC_FILES res/res.qrc)
set (PLUGIN_SRCS
#        screen-casting-request.cpp
#        screen-casting.cpp
#        pipewire-core.cpp
#        pipewire-source-stream.cpp
#        pipewire-source-item.cpp
        window-thumbnail-plugin.cpp
        x-window-thumbnail.cpp
        window-thumbnail-config.cpp
        window-thumbnail-config.h
        mpris/mpris-player-collecter.cpp
        mpris/mpris-player-collecter.h
        mpris/player-item.cpp
        mpris/player-item.h
        mpris/player-items-model.cpp
        mpris/player-items-model.h
        mpris/window-thumbnail-mpris-model.cpp
        mpris/window-thumbnail-mpris-model.h
        mpris/properties.h
        core/thumbnail-item.cpp
        core/thumbnail-item.h
        core/thumbnail.h
        core/thumbnail-buffer.h
        core/thumbnail-buffer.h
        core/wlcom-thumbnail.cpp
        core/wlcom-thumbnail.h
        core/window-thumbnail-item.cpp
        core/window-thumbnail-item.h
        core/graphics-memory-buffer.cpp
        core/graphics-memory-buffer.h
        core/discard-texture-resources-runnable.h
        core/workspace-thumbnail-item.cpp
        core/workspace-thumbnail-item.h
        core/scene-graph-resource-release-queue.cpp
        core/scene-graph-resource-release-queue.h
)
#generate wayland protocol files
#ecm_add_qtwayland_client_protocol(PLUGIN_SRCS
#        PROTOCOL ${PLASMA_WAYLAND_PROTOCOLS_DIR}/screencast.xml
#        BASENAME zkde-screencast-unstable-v1
#)
set_source_files_properties(
        mpris/org.freedesktop.DBus.Properties.xml
        mpris/org.mpris.MediaPlayer2.Player.xml
        mpris/org.mpris.MediaPlayer2.xml
        PROPERTIES
        NO_NAMESPACE ON)
if(COMMAND qt_add_dbus_interface)
    qt_add_dbus_interface(PLUGIN_SRCS mpris/org.freedesktop.DBus.Properties.xml dbusproperties)
    qt_add_dbus_interface(PLUGIN_SRCS mpris/org.mpris.MediaPlayer2.xml mprisplayer2)
    qt_add_dbus_interface(PLUGIN_SRCS mpris/org.mpris.MediaPlayer2.Player.xml mprisplayer2player)
else()
    qt5_add_dbus_interface(PLUGIN_SRCS mpris/org.freedesktop.DBus.Properties.xml dbusproperties)
    qt5_add_dbus_interface(PLUGIN_SRCS mpris/org.mpris.MediaPlayer2.xml mprisplayer2)
    qt5_add_dbus_interface(PLUGIN_SRCS mpris/org.mpris.MediaPlayer2.Player.xml mprisplayer2player)
endif()

add_library(${PROJECT_NAME} SHARED ${PLUGIN_SRCS} ${QRC_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE ../../platform)

set(PLUGIN_IMPORT_URI "org.ukui.windowThumbnail")
target_compile_definitions(${PROJECT_NAME} PRIVATE PLUGIN_IMPORT_URI="${PLUGIN_IMPORT_URI}")

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Quick
        Qt::GuiPrivate
        Qt${QT_VERSION_MAJOR}::DBus
        ${WINDOW_THUMBNAIL_PLUGIN_EXTERNAL_LIBS}
        KF${KF_VERSION_MAJOR}::WindowSystem
        ukui-quick::platform
    )
if (QT_VERSION_MAJOR EQUAL "5")
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt::X11Extras Qt5::Gui_EGL KF5::WaylandClient)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt::GuiPrivate Plasma::KWaylandClient)
endif()

set(PLUGIN_INSTALL_PATH "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/qt${QT_VERSION_MAJOR}/qml/org/ukui/windowThumbnail")
install(DIRECTORY "qml" DESTINATION "${PLUGIN_INSTALL_PATH}")
install(FILES qmldir DESTINATION ${PLUGIN_INSTALL_PATH})
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})

if(BUILD_TEST)
    add_subdirectory(autotest)
endif ()
if(BUILD_TEST AND BUILD_INTERACTIVE_TESTS)
    add_subdirectory(core/test)
endif ()

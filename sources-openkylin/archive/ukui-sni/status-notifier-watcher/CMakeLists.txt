project(ukui-sni-watcher VERSION 1.0 LANGUAGES C CXX)
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_MICRO 0)
set(UKUI_SNI_WATCHER_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO})
set(3rdParties_DIR  ../3rd-parties/qtsingleapplication/src/)
configure_file (ukui-sni-watcher-config.h.in ukui-sni-watcher-config.h @ONLY)
include_directories(../common)
include_directories(../)
set(ukuiSNIWatcher_SRCS
        ${3rdParties_DIR}qtsinglecoreapplication.h
        ${3rdParties_DIR}qtsinglecoreapplication.cpp
        ${3rdParties_DIR}qtlocalpeer.h
        ${3rdParties_DIR}qtlocalpeer.cpp
        watcher.cpp
        watcher.h
        watcher-private.h
        status-notifier-watcher-application.cpp
        status-notifier-watcher-application.h
        ../log-utils.cpp ../log-utils.h
        main.cpp)
set_source_files_properties(../dbus/org.kde.StatusNotifierWatcher.xml PROPERTIES
        NO_NAMESPACE false
        INCLUDE "./watcher-private.h"
        CLASSNAME StatusNotifierWatcherAdaptor
)
if(COMMAND qt_add_dbus_adaptor)
    qt_add_dbus_adaptor(ukuiSNIWatcher_SRCS ../dbus/org.kde.StatusNotifierWatcher.xml watcher-private.h WatcherPrivate)
else()
    qt5_add_dbus_adaptor(ukuiSNIWatcher_SRCS ../dbus/org.kde.StatusNotifierWatcher.xml watcher-private.h WatcherPrivate)
endif()
set(STATUS_NOTIFIER_ITEM_XML ../dbus/org.kde.StatusNotifierItem.xml)
set_source_files_properties(${STATUS_NOTIFIER_ITEM_XML} PROPERTIES
        NO_NAMESPACE false
        INCLUDE "../common/typedefs.h"
        CLASSNAME OrgKdeStatusNotifierItemInterface
        )
if(COMMAND qt_add_dbus_interface)
    qt_add_dbus_interface(ukuiSNIWatcher_SRCS ../dbus/org.kde.StatusNotifierItem.xml statusnotifieriteminterface)
else()
    qt5_add_dbus_interface(ukuiSNIWatcher_SRCS ../dbus/org.kde.StatusNotifierItem.xml statusnotifieriteminterface)
endif()

add_executable(${PROJECT_NAME} ${ukuiSNIWatcher_SRCS})
target_include_directories(${PROJECT_NAME} PUBLIC ${3rdParties_DIR})

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::Gui
        )
install(TARGETS ${PROJECT_NAME} DESTINATION /usr/bin)
install(FILES ukui-sni-watcher.desktop DESTINATION /etc/xdg/autostart)
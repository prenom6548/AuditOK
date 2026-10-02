set(CMAKE_CXX_STANDARD 20)
include(../cmake/find-modules/FindXCB.cmake)
set(XCB_LIBS
        xcb
        xcb-util
        xcb-damage
        xcb-image
        xcb-composite
        xcb-shape
        )
if (QT_VERSION_MAJOR EQUAL 6)
    find_package(KF6WindowSystem REQUIRED)
elseif (QT_VERSION_MAJOR EQUAL 5)
    find_package(KF5WindowSystem REQUIRED)
endif()

include_directories(../common)
include_directories(../)
set(XEMBED_SNI_PROXY_SOURCES
        main.cpp
        fdo-selection-manager.cpp
        fdo-selection-manager.h
        sni-proxy.cpp
        sni-proxy.h
        xtest-sender.cpp
        xtest-sender.h
        xcb-utils.h
        ../common/dbus-types.cpp
        ../common/dbus-types.h
        ../common/typedefs.h
        c_ptr.h
        ../log-utils.cpp
        ../log-utils.h
        )

qt_add_dbus_adaptor(XEMBED_SNI_PROXY_SOURCES ../dbus/org.kde.StatusNotifierItem.xml sni-proxy.h SNIProxy)
set(statusnotifierwatcher_xml ../dbus/org.kde.StatusNotifierWatcher.xml)
set_source_files_properties(${statusnotifierwatcher_xml} PROPERTIES
        NO_NAMESPACE false
        INCLUDE "../common/dbus-types.h"
)
qt_add_dbus_interface(XEMBED_SNI_PROXY_SOURCES ${statusnotifierwatcher_xml} statusnotifierwatcher_interface)

add_executable(xembed-sni-proxy ${XEMBED_SNI_PROXY_SOURCES})
target_include_directories(xembed-sni-proxy PUBLIC ../common)

set_package_properties(XCB PROPERTIES TYPE REQUIRED)

target_link_libraries(xembed-sni-proxy
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Gui
        KF${QT_VERSION_MAJOR}::WindowSystem
        ${XCB_LIBS}
        X11::Xtst
        )
if (QT_VERSION_MAJOR EQUAL 5)
    target_link_libraries(xembed-sni-proxy Qt5::X11Extras)
else()
    target_link_libraries(xembed-sni-proxy Qt::GuiPrivate)
endif()

install(TARGETS xembed-sni-proxy DESTINATION /usr/bin)
install(FILES xembed-sni-proxy.desktop DESTINATION /etc/xdg/autostart)

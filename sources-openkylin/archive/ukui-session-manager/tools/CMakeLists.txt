find_package(PkgConfig)
find_package(KF5Screen)
find_package(KF5Wayland)

include_directories(${KF5Wayland_LIBRARIES})
pkg_check_modules(KScreen REQUIRED kscreen2)

set(tools_SRCS
    main.cpp
    ukuipower.cpp
    powerprovider.cpp
    mainwindow.h
    mainwindow.cpp
    grab-x11.cpp
    grab-x11.h
    xeventmonitor.cpp
    xeventmonitor.h
    loginedusers.h
    lockchecker.h
    lockchecker.cpp
    loginedusers.cpp
    ../ukui-session/xdgdirs.cpp
    ../ukui-session/xdgdesktopfile.cpp
    myiconlabel.cpp
    myiconlabel.h
    mypushbutton.cpp
    mypushbutton.h
    pushbutton.cpp
    pushbutton.h
    commonpushbutton.cpp
    commonpushbutton.h
    plasma-shell-manager.cpp
    plasma-shell-manager.h
    mylistview.cpp
    mylistview.h
    ukuilockinfo.h
    ukuilockinfo.cpp
    ../ukui-session/xdgdesktopfile.h
    ../ukui-session/xdgdesktopfile.cpp
    ../ukui-session/xdgdirs.h
    ../ukui-session/xdgdirs.cpp
    ../ukui-session/xdgmacros.h
    systemupgradecheck.cpp
    systemupgradecheck.h
)

add_executable(ukui-session-tools ${tools_SRCS})

find_package(PkgConfig)
pkg_search_module(GSETTINGS_QT REQUIRED gsettings-qt)
include_directories(${GSETTINGS_QT_INCLUDES})

pkg_check_modules(KYSDKSYSTIME_PKG kysdk-systime)
target_include_directories(ukui-session-tools PRIVATE ${KYSDKSYSTIME_PKG_INCLUDE_DIRS})
target_link_directories(ukui-session-tools PRIVATE ${KYSDKSYSTIME_PKG_LIBRARY_DIRS})

pkg_check_modules(KDKINFO REQUIRED kysdk-sysinfo)
include_directories(${KDKINFO_INCLUDE_DIRS})
link_directories(${KDKINFO_LIBRARY_DIRS})

pkg_check_modules(GLIB2 REQUIRED glib-2.0)
pkg_check_modules(GIO2 REQUIRED gio-2.0)

include_directories(${GLIB2_INCLUDE_DIRS})
include_directories(${GIO2_INCLUDE_DIRS})



target_link_libraries(ukui-session-tools
    Qt5::Widgets
    Qt5::Core
    Qt5::Gui
    Qt5::DBus
    Qt5::Multimedia
    Qt5::X11Extras
    ${GSETTINGS_QT_LIBRARIES}
    ${KYSDKSYSTIME_PKG_LIBRARIES}
    ${KDKINFO_LIBRARIES}
    ${KF5Wayland_LIBRARIES}
    ${GLIB2_LIBRARIES}
    ${GIO2_LIBRARIES}
    -lxcb
    -lX11
    -lXtst
    -lukui-log4qt
    KF5::WaylandServer
    KF5::WaylandClient
    KF5::Screen
    KF5::WindowSystem
)

install(TARGETS
        ukui-session-tools
        DESTINATION bin)

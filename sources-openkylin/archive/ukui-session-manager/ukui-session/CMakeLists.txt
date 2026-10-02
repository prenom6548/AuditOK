# Populate a CMake variable with the sources
set(ukui-session_SRCS
    main.cpp
    sessionapplication.cpp
    modulemanager.cpp
    ukuimodule.cpp
    idlewatcher.cpp
    sessiondbusadaptor.h
    idlewatcher.cpp
    idlewatcher.h
    idleadbusdaptor.h
    xdgautostart.cpp
    xdgautostart.h
    xdgdesktopfile.cpp
    xdgdesktopfile.h
    xdgdirs.cpp
    xdgdirs.h
    xdgmacros.h
    usminhibit.cpp
    usminhibit.h
    ../tools/ukuipower.cpp
    ../tools/powerprovider.cpp
    ../tools/lockchecker.cpp
    ../tools/lockchecker.h
    ../tools/loginedusers.cpp
    ../tools/loginedusers.h
    ukuisessiondebug.cpp
    ukuisessiondebug.h
    sessionmanagercontext.h
    sessionmanagercontext.cpp
    ukuilockinfo.cpp
    simpledeskapplication.cpp
    simpledeskapplication.h
    freedesktophelper.cpp
    freedesktophelper.h
    abnormalwindow.cpp
    usdblockshortcut.h
    usdblockshortcut.cpp
    screensaverproxy.h
    screensaverproxy.cpp
)

#find_package(QT5XDG )
find_package(PkgConfig)
pkg_search_module(GSETTINGS_QT REQUIRED gsettings-qt)
include_directories(${GSETTINGS_QT_INCLUDES})

pkg_check_modules(GLIB2 REQUIRED glib-2.0)
pkg_check_modules(GIO2 REQUIRED gio-2.0)

pkg_search_module(X11 REQUIRED x11)
pkg_check_modules(KDKINFO REQUIRED kysdk-sysinfo)
pkg_check_modules(KDKPACKAGE REQUIRED kysdk-package)

find_package(ukui-quick COMPONENTS core framework platform REQUIRED)
pkg_check_modules(KYSDKWINDOWHELPER REQUIRED kysdk-ukuiwindowhelper)

find_package(PolkitQt5-1 REQUIRED)

include_directories(${X11_INCLUDES})
include_directories(${GLIB2_INCLUDE_DIRS})
include_directories(${GIO2_INCLUDE_DIRS})
include_directories(${KDKINFO_INCLUDE_DIRS})
include_directories(${KDKPACKAGE_INCLUDE_DIRS})
include_directories(${KYSDKWINDOWHELPER_INCLUDE_DIRS})

link_directories(
    ${KDKINFO_LIBRARY_DIRS}
    ${KDKPACKAGE_LIBRARY_DIRS}
    ${KYSDKWINDOWHELPER_LIBRARY_DIRS}
)

add_executable(ukui-session ${ukui-session_SRCS})
target_link_libraries(ukui-session
    Qt5::Widgets
    Qt5::Core
    Qt5::DBus
    Qt5::Multimedia
    KF5::IdleTime
    KF5::CoreAddons
    KF5::ConfigCore
    KF5::ConfigGui
    KF5::WindowSystem
    Qt5::Quick
    ukui-quick::framework
    ukui-quick::core
    ukui-quick::platform
    PolkitQt5-1::Core
    -lSM -lICE
    ${GSETTINGS_QT_LIBRARIES} ${X11_LIBRARIES}
    ${GLIB2_LIBRARIES}
    ${GIO2_LIBRARIES}
    ${KDKINFO_LIBRARIES}
    ${KDKPACKAGE_LIBRARIES}
    ${KYSDKWINDOWHELPER_LIBRARIES}
)
#${GSETTINGS_QT_LIBRARIES}

install(TARGETS ukui-session DESTINATION bin)

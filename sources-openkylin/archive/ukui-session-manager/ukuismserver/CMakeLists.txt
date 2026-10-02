set(UKUISMSERVER_SRCS
    main.cpp
    ukuismclient.cpp
    ukuismclient.h
    ukuismserver.cpp
    ukuismserver.h
    ukuismserverdebug.cpp
    ukuismserverdebug.h
    ukuismconnection.h
    ukuismserverdbusadaptor.cpp
    ukuismserverdbusadaptor.h
    ../ukui-session/xdgdesktopfile.h
    ../ukui-session/xdgdesktopfile.cpp
    ../ukui-session/xdgdirs.h
    ../ukui-session/xdgdirs.cpp
)

qt5_add_dbus_interface(UKUISMSERVER_SRCS org.ukui.KWin.Session.xml ukuikwinsession_interface)

add_executable(ukuismserver ${UKUISMSERVER_SRCS})
target_link_libraries(ukuismserver PRIVATE Qt5::Widgets Qt5::DBus KF5::WindowSystem KF5::ConfigCore
                      KF5::ConfigGui KF5::CoreAddons -lSM -lICE)

install(TARGETS ukuismserver DESTINATION bin)

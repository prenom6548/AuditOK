QT += core gui dbus x11extras
CONFIG += c++11 no_keywords x11extras link_pkgconfig xrandr
CONFIG -= app_bundle

INCLUDEPATH += -I $$PWD/

PLUGIN_INSTALL_DIRS = $$[QT_INSTALL_LIBS]/ukui-settings-daemon

EXTRA_CFLAGS +=-Wno-date-time -Werror=<return-type>
QMAKE_CXXFLAGS+=-Werror=return-type
PKGCONFIG += glib-2.0  gio-2.0 libxklavier x11 xrandr xtst atk gdk-3.0 gtk+-3.0 xi gudev-1.0 kscreen2

QMAKE_CXXFLAGS +=  -Wno-unused-parameter
SOURCES += \
        $$PWD/abstract-manager.cpp \
        $$PWD/abstract_touch_calibrate.cpp \
        $$PWD/clib-syslog.c             \
        $$PWD/QGSettings/qconftype.cpp  \
        $$PWD/QGSettings/qgsettings.cpp \
        $$PWD/rfkillswitch.cpp \
        $$PWD/touch-calibrate.cpp \
        $$PWD/touch_calibrate_wayland.cpp \
        $$PWD/touch_calibrate_x.cpp \
        $$PWD/usd_base_class.cpp \
        $$PWD/usdqjsonconf.cpp \
        $$PWD/xeventmonitor.cpp         \
        $$PWD/eggaccelerators.c         \
        $$PWD/ukui-input-helper.c       \
        $$PWD/ukui-keygrab.cpp

HEADERS += \
        $$PWD/abstract-manager.h \
        $$PWD/abstract_touch_calibrate.h \
        $$PWD/clib-syslog.h             \
        $$PWD/dbus-security.h \
        $$PWD/plugin-interface.h        \
        $$PWD/QGSettings/qconftype.h    \
        $$PWD/QGSettings/qgsettings.h   \
        $$PWD/rfkillswitch.h \
        $$PWD/touch-calibrate.h \
        $$PWD/touch_calibrate_wayland.h \
        $$PWD/touch_calibrate_x.h \
        $$PWD/usd_base_class.h \
        $$PWD/usd_global_define.h \
        $$PWD/usdqjsonconf.h \
        $$PWD/xeventmonitor.h           \
        $$PWD/eggaccelerators.h         \
        $$PWD/ukui-input-helper.h       \
        $$PWD/ukui-keygrab.h            \
        $$PWD/config.h \
        $$PWD/xrandroutput.h    \
        $$PWD/plugin-manager-interface.h

RESOURCES += \
    $$PWD/ukui_icon.qrc

QT       += core dbus concurrent
QT       -= gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = launchSysDbus
TEMPLATE = lib

CONFIG += console c++11 link_pkgconfig plugin
CONFIG -= app_bundle

PKGCONFIG     += gio-2.0 \
                 gio-unix-2.0 \
                 kysdk-sysinfo \
                 kysdk-package

LIBS          += -L$$[QT_INSTALL_LIBS] -lpolkit-qt5-core-1 -lukuiserviceinterface


DESTDIR = .
INCLUDEPATH += .

inst1.files += conf/com.control.center.qt.systemdbus.service
inst1.path = /usr/share/dbus-1/system-services/
inst2.files += conf/com.control.center.qt.systemdbus.conf
inst2.path = /etc/dbus-1/system.d/
inst3.files += conf/com.control.center.qt.systemdbus.policy
inst3.path = /usr/share/polkit-1/actions/

limit1.files = conf/com.control.center.qt.systemdbus.limit
limit1.path = /etc/dbus-1/conf/
limit2.files = conf/com.control.center.qt.systemdbus.limit
limit2.path = /usr/share/dbus-1/conf/

verify1.files = conf/com.control.center.qt.systemdbus.limit.verify
verify1.path = /etc/dbus-1/conf/
verify2.files = conf/com.control.center.qt.systemdbus.limit.verify
verify2.path = /usr/share/dbus-1/conf/

yaml.path = /etc/kylin-config/basic/
yaml.files = conf/com.control.center.qt.systemdbus.yaml

#target.source += $$TARGET
target.path = /usr/libexec/ukui-services/system

INSTALLS += \
    target \
    inst1 \
    inst2 \
    inst3 \
    limit1 \
    limit2 \
    verify1 \
    verify2 \
    yaml

DISTFILES += metadata.json

HEADERS += \
    freedesktophelper.h \
    sysdbusregister.h \
    sysdbusregisterservice.h

SOURCES += \
    freedesktophelper.cpp \
    main.cpp \
    sysdbusregister.cpp \
    sysdbusregisterservice.cpp

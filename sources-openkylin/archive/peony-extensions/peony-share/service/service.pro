TEMPLATE = app
TARGET = peony-samba-service

QT += core dbus

PKGCONFIG += gio-2.0 polkit-gobject-1
CONFIG += link_pkgconfig c++11 no_keywords

DEFINES += DBUS_NAME=\\\"org.ukui.samba.share.config\\\"
DEFINES += DBUS_PATH=\\\"/org/ukui/samba/share\\\"

SOURCES += $$PWD/peony-samba-service.cpp  $$PWD/samba-config.cpp
HEADERS += $$PWD/samba-config.h
target.path = /usr/libexec/

INSTALLS += target

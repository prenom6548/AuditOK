#-------------------------------------------------
#
# Project created by QtCreator 2019-06-29T11:18:10
#
#-------------------------------------------------

QT       += core gui dbus

greaterThan(QT_MAJOR_VERSION, 4):QT += widgets

TARGET = ukui-upower
TEMPLATE = app

CONFIG += c++14 link_pkgconfig
PKGCONFIG += gsettings-qt

LIBS += -lukui-log4qt

SOURCES += main.cpp\
    batterydbus.cpp \
    batteryinfo/batteryinfo.cpp \
    dbus.cpp \
    upowerdbus.cpp \
    upowerinfo/upowerinfo.cpp

HEADERS  += batteryinfo/batteryinfo.h \
    batterydbus.h \
    common/common.h \
    dbus.h \
    upowerdbus.h \
    upowerinfo/upowerinfo.h


# Default rules for deployment.
qnx: target.path = /tmp/usr/bin
else: unix:!android: target.path = /usr/bin
!isEmpty(target.path): INSTALLS += target

desktop.files += resources/ukui-upower.desktop
desktop.path = /etc/xdg/autostart/
INSTALLS += desktop


xorg.files += resources/10-monitor.conf
xorg.path = /usr/share/X11/xorg.conf.d/
INSTALLS += xorg

conf.files += resources/org.ukui.power-manager.gschema.xml
conf.path=/usr/share/glib-2.0/schemas/
INSTALLS += conf

cf.files += resources/power-manager.conf
cf.path = /etc/ukui/power
INSTALLS += cf

dbus_service.files += resources/org.ukui.upower.service
dbus_service.path = /usr/share/dbus-1/services/
INSTALLS += dbus_service
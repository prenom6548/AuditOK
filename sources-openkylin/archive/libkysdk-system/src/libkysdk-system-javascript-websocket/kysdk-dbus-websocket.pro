#-------------------------------------------------
#
# Project created by QtCreator 2021-11-09T23:11:59
#
#-------------------------------------------------

QT       += core gui network  concurrent dbus webchannel websockets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = kysdk-dbus-websocket
TEMPLATE = app

CONFIG += c++11 console

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

desktop.path = /etc/xdg/autostart/
desktop.files = kysdk-dbus-websocket.desktop

INSTALLS += desktop 

SOURCES += \
    kysdk/bios.cpp \
    kysdk/cpuinfo.cpp \
    kysdk/disk.cpp \
    kysdk/gps.cpp \
    kysdk/kysdkdbus.cpp \
    main.cpp \
    kysdk/mainboard.cpp \
    kysdk/net.cpp \
    kysdk/netcard.cpp \
    kysdk/netlink.cpp \
    kysdk/packageinfo.cpp \
    kysdk/peripheralsenum.cpp \
    kysdk/print.cpp \
    kysdk/process.cpp \
    kysdk/resolutionctl.cpp \
    kysdk/resource.cpp \
    kysdk/runinfo.cpp \
    kysdk/sysinfo.cpp \
    shared/websocketclientwrapper.cpp \
    shared/websockettransport.cpp 

HEADERS += \
    kysdk/bios.h \
    kysdk/cpuinfo.h \
    kysdk/dependence.h \
    kysdk/disk.h \
    kysdk/gps.h \
    kysdk/kysdkdbus.h \
    kysdk/mainboard.h \
    kysdk/net.h \
    kysdk/netcard.h \
    kysdk/netlink.h \
    kysdk/packageinfo.h \
    kysdk/peripheralsenum.h \
    kysdk/print.h \
    kysdk/process.h \
    kysdk/resolutionctl.h \
    kysdk/resource.h \
    kysdk/runinfo.h \
    kysdk/sysinfo.h \
    shared/websocketclientwrapper.h \
    shared/websockettransport.h 



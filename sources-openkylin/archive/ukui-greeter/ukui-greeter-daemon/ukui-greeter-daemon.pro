QT -= gui
QT += core dbus

CONFIG += c++11 dbus
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
        serviceinterface.cpp

# Default rules for deployment.
TARGET = ukui-greeter-daemon
target.path = /usr/lib/ukui-greeter/

configfile.files = org.ukui.GreeterDaemon.conf
configfile.path = /etc/dbus-1/system.d/

services.files = org.ukui.GreeterDaemon.service
services.path = /usr/share/dbus-1/system-services/

kconf2.files = org.ukui.GreeterDaemon.yaml
kconf2.path = /usr/share/ukui-greeter/kylin-config/

INSTALLS += target configfile services kconf2

DISTFILES += \
    org.ukui.GreeterDaemon.conf \
    org.ukui.GreeterDaemon.service

HEADERS += \
    serviceinterface.h

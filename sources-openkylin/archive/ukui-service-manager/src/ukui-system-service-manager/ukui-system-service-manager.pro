QT -= gui
QT += dbus

TARGET = ukui-system-service-manager

CONFIG += c++11 console
CONFIG -= app_bundle

include(../libukuiservice/libukuiservice.pri)

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
        systemserviceregister.cpp

DISTFILES += \
    ../../data/system/org.ukui.serviceManager.service \
    ../../data/org.ukui.serviceManager.conf \
    ../../data/ukui-system-service-manager.service \
    register-ukui-system-service

HEADERS += \
    systemserviceregister.h

target.path = /usr/bin
INSTALLS += target

conf.path = /usr/share/dbus-1/system.d
conf.files = ../../data/org.ukui.serviceManager.conf

INSTALLS += conf

dbusservice.path = /usr/share/dbus-1/system-services/
dbusservice.files += ../../data/system/org.ukui.serviceManager.service
INSTALLS += dbusservice

systemdservice.path = /usr/lib/systemd/system
systemdservice.files += ../../data/ukui-system-service-manager.service
INSTALLS += systemdservice

registerhelper.path = /usr/bin
registerhelper.files += register-ukui-system-service
INSTALLS += registerhelper

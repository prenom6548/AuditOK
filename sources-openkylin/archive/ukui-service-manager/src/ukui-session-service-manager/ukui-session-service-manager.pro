QT -= gui
QT += dbus

CONFIG += c++11 console
CONFIG -= app_bundle

TARGET = ukui-session-service-manager

include(../libukuiservice/libukuiservice.pri)

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
        sessionserviceregister.cpp

HEADERS += \
    sessionserviceregister.h

target.path = /usr/bin
INSTALLS += target

DISTFILES += \
    ../../data/session/org.ukui.serviceManager.service \
    ../../data/ukui-session-service-manager.service \
    register-ukui-session-service

dbusservice.path = /usr/share/dbus-1/services
dbusservice.files += ../../data/session/org.ukui.serviceManager.service
INSTALLS += dbusservice

systemdservice.path = /usr/lib/systemd/user
systemdservice.files += ../../data/ukui-session-service-manager.service
INSTALLS += systemdservice

registerhelper.path = /usr/bin
registerhelper.files += register-ukui-session-service
INSTALLS += registerhelper

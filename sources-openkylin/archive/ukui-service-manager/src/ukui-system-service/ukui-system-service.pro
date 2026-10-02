QT -= gui
QT += dbus

TARGET = ukui-system-service

CONFIG += c++11 console
CONFIG -= app_bundle

include(../libukuiservice/libukuiservice.pri)

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
        systemservicelauncher.cpp

HEADERS += \
    systemservicelauncher.h

target.path = /usr/bin
INSTALLS += target

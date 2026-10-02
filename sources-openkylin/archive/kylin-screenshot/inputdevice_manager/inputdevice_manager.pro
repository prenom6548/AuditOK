# QT -= gui

QT += gui dbus xml

QT += widgets

CONFIG += c++11 console
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
        input_device_manager.cpp \
        main.cpp

# installs
unix:!macx {
    isEmpty(PREFIX) {
      packaging {
        PREFIX = /usr
      } else {
        PREFIX = /usr/local
      }
    }

    target.path = $${PREFIX}/bin/

    dbus.path = $${PREFIX}//share/dbus-1/system-services
    dbus.files = com.kylin.inputdevice.manager.service

    dbus_conf.path = $${PREFIX}//share/dbus-1/system.d
    dbus_conf.files = com.kylin.inputdevice.manager.conf

    INSTALLS  += target dbus dbus_conf
}

HEADERS += \
    input_device_manager.h
unix|win32: LIBS += -linput

unix|win32: LIBS += -ludev

DBUS_ADAPTORS += com.kylin.inputdevice.manager.xml


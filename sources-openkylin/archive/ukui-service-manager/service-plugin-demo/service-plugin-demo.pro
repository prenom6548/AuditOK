QT += gui dbus

TARGET = service-plugin-demo

TEMPLATE = lib
CONFIG += plugin

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    demoservice.cpp \
    demoservice2.cpp \
    demoserviceplugin.cpp

HEADERS += \
    demoservice.h \
    demoservice2.h \
    demoserviceplugin.h

LIBS += -L../ukuiserviceinterface -lukuiserviceinterface

DISTFILES += service-plugin-demo.json \
    org.ukui.service.demo.conf

DESTDIR = $$PWD

target.path = /usr/local/lib
INSTALLS += target

conf.path = /usr/share/dbus-1/system.d
conf.files = org.ukui.service.demo.conf
INSTALLS += conf

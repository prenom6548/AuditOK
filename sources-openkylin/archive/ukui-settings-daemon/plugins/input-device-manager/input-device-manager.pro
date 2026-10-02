QT += gui core

TEMPLATE = lib

TARGET = input-device-manager

CONFIG += c++11 plugin

include($$PWD/../../common/common.pri)

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

DEFINES += INPUTDEVICEMANAGER_LIBRARY MODULE_NAME=\\\"input-device-manager\\\"

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    device-monitor.cpp \
    input-device-factory.cpp \
    input-device-function.cpp \
    input-device-helper.cpp \
    input-device-manager.cpp \
    input-device.cpp \
    input-monitor.cpp \
    input-plugin.cpp  \
    input-gsettings.cpp \
    input-process-settings.cpp \
    input-wayland-device.cpp \
    input-wlcom-device.cpp \
    input-wlcom-helper.cpp \
    input-x-device.cpp
HEADERS += \
    device-monitor.h \
    input-common.h \
    input-device-factory.h \
    input-device-function.h \
    input-device-helper.h \
    input-device-manager.h \
    input-device.h \
    input-monitor.h \
    input-plugin.h \
    input-gsettings.h \
    input-process-settings.h \
    input-wayland-device.h \
    input-wlcom-device.h \
    input-wlcom-helper.h \
    input-x-device.h \
    wlcom-config.h
# Default rules for deployment.
input-device-manager_lib.path = $${PLUGIN_INSTALL_DIRS}
input-device-manager_lib.files = $$OUT_PWD/libinput-device-manager.so

devicefile.path = /opt/system/conf/usd/
devicefile.files = $$PWD/conf/deviceType.json

INSTALLS += input-device-manager_lib devicefile

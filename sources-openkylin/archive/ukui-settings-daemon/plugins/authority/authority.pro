QT -= gui
QT += core dbus x11extras
CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS MODULE_NAME=\\\"usd-authority\\\"

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

TEMPLATE = app
TARGET = authoritydbus
DESTDIR = .

LIBS += -lddcutil -lxcb -lxcb-randr -ludev -levdev

target.source += $$TARGET
target.path = /usr/bin
limityaml.files += conf/com.kylin.ukui.SettingsDaemon.yaml
limityaml.path = /usr/share/ukui-settings-daemon/
service.files += conf/com.kylin.ukui.SettingsDaemon.service
service.path = /usr/share/dbus-1/system-services/
conffile.files += conf/com.kylin.ukui.SettingsDaemon.conf
conffile.path = /etc/dbus-1/system.d/
securityfile.path = /opt/system/conf/usd/
securityfile.files += security/SecurityConf.ini
touchpadsupport.path = /opt/system/conf/ukui-security-support/
touchpadsupport.files += security/org.ukui.peripherals-touchpad.json
mediakeysupport.path = /opt/system/conf/ukui-security-support/
mediakeysupport.files += security/org.ukui.SettingsDaemon.plugins.customized.json
poweroptionsupport.path = /opt/system/conf/ukui-security-support/
poweroptionsupport.files += security/org.ukui.SettingsDaemon.plugins.globalManager.json

optupdatefile.path = /opt/system/resource/update-manifests/
optupdatefile.files = security/ukui-settings-daemon.json

INCLUDEPATH += /usr/include/libevdev-1.0

INSTALLS +=  \
            target \
            service \
            conffile \
            limityaml \
            securityfile \
            touchpadsupport \
            mediakeysupport \
            poweroptionsupport \
            optupdatefile


SOURCES += \
        authorityservice.cpp \
        brightnesshelper.cpp \
        camera-helper.cpp \
        device-manager.cpp \
        globalconfig.cpp \
        save-security-config.cpp    \
#        procaddserver.cpp   \
        inputeventmonitor.cpp  \
        main.cpp


# Default rules for deployment.
#qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /opt/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target

HEADERS += \
    authorityservice.h \
    brightnesshelper.h \
    device-manager.h \
    globalconfig.h \
    authority-base.h    \
    save-security-config.h  \
#    procaddserver.h \
    inputeventmonitor.h  \
    camera-helper.h


DISTFILES += \
    conf/com.kylin.ukui.SettingsDaemon.conf \
    conf/com.kylin.ukui.SettingsDaemon.service


include($$PWD/../../common/common.pri)

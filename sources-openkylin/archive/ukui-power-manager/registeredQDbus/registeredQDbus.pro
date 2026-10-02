QT       += core dbus x11extras
QT       -= gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ukui-powermanagement-service
TEMPLATE = app

CONFIG += console c++11 link_pkgconfig
CONFIG -= app_bundle

LIBS += -lukui-log4qt -lpci -ludev -linput

DESTDIR = .
INCLUDEPATH += .
INCLUDEPATH += $$PWD/../shared/include
#DEFINES += QT_NO_DEBUG_OUTPUT
DEFINES += MODULE_NAME=\\\"pm\\\"

upmDbusAuthorityControlEtc.files += conf/org.ukui.powermanagement.yaml
upmDbusAuthorityControlEtc.path = /usr/share/ukui/ukui-power-manager/
INSTALLS += upmDbusAuthorityControlEtc


inst1.files += conf/org.ukui.powermanagement.service
inst1.path = /usr/share/dbus-1/system-services/
inst2.files += conf/org.ukui.powermanagement.conf
inst2.path = /etc/dbus-1/system.d/

target.source += $$TARGET
target.path = /usr/bin
INSTALLS += target inst1 inst2

upmHardwareGlobalConf.files += conf/upm-hardware-global.conf
upmHardwareGlobalConf.path = /opt/system/resource/ukui/ukui-power-manager/

upmHardwareExtendConf.files += conf/extend-config/Lenovo.config
upmHardwareExtendConf.files += conf/extend-config/KaiTian.config
upmHardwareExtendConf.files += conf/extend-config/bxc.config
upmHardwareExtendConf.files += conf/extend-config/hp.config
upmHardwareExtendConf.files += conf/extend-config/GreatWall.config
upmHardwareExtendConf.files += conf/extend-config/D525.config
upmHardwareExtendConf.files += conf/extend-config/JH3KMA01.config
upmHardwareExtendConf.files += conf/extend-config/Arch.config

upmHardwareExtendConf.path = /opt/system/resource/ukui/ukui-power-manager/upm-hardware-extend.d/

INSTALLS += upmHardwareGlobalConf upmHardwareExtendConf

HEADERS += \
    brightness-control/brightnessnode.h \
    cpu-control/upmcpucontrol.h \
    cpu-control/upmcpucoreinfo.h \
    cpu-control/upmcpuoccupy.h \
    dev-power-control/devfreqpolicyrpm.h \
    dev-power-control/devpowercontrol.h \
    dev-power-control/devrpminterface.h \
    dev-power-control/netwolinfo.h \
    dev-power-control/netwolrpm.h \
    dev-power-control/satalinkrpm.h \
    dev-power-control/wakesourcerpm.h \
    gpu-power-control/gpupowercontrol.h \
    powerconfig.h \
    sysdbusregister.h \
    dev-power-control/devicepowerinfo.h \
    dev-power-control/i2crpm.h \
    dev-power-control/pcirpm.h \
    dev-power-control/usbrpm.h \
    dev-power-control/pcieaspm.h \
    dev-power-control/keyboardbacklight.h \
    dev-power-control/touchscreen.h \
    dev-power-control/audiorpm.h \
    syslog-handler.hpp \
    upmcommonfile.h \
    upmcustomhardwareconfig.h \
    hardware-reader/hardwarereader.h

SOURCES += \
    brightness-control/brightnessnode.cpp \
    cpu-control/upmcpucontrol.cpp \
    cpu-control/upmcpucoreinfo.cpp \
    cpu-control/upmcpuoccupy.cpp \
    dev-power-control/audiorpm.cpp \
    dev-power-control/devfreqpolicyrpm.cpp \
    dev-power-control/devpowercontrol.cpp \
    dev-power-control/devrpminterface.cpp \
    dev-power-control/netwolinfo.cpp \
    dev-power-control/netwolrpm.cpp \
    dev-power-control/satalinkrpm.cpp \
    dev-power-control/wakesourcerpm.cpp \
    gpu-power-control/gpupowercontrol.cpp \
    main.cpp \
    powerconfig.cpp \
    sysdbusregister.cpp \
    dev-power-control/devicepowerinfo.cpp \
    dev-power-control/i2crpm.cpp \
    dev-power-control/pcirpm.cpp \
    dev-power-control/usbrpm.cpp \
    dev-power-control/pcieaspm.cpp \
    dev-power-control/keyboardbacklight.cpp \
    dev-power-control/touchscreen.cpp \
    upmcommonfile.cpp \
    upmcustomhardwareconfig.cpp \
    hardware-reader/hardwarereader.cpp


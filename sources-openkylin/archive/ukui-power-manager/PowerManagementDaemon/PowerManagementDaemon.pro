QT       += core gui dbus network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ukui-powermanagement

CONFIG += c++11 link_pkgconfig

PKGCONFIG += gsettings-qt kysdk-soundeffects kysdk-sysinfo

LIBS += -lukui-log4qt

INCLUDEPATH += $$PWD/../shared/include

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
TRANSLATIONS+=\
    translations/ukui-power-manager-daemon_ar.ts \
    translations/ukui-power-manager-daemon_ky.ts \
    translations/ukui-power-manager-daemon_th.ts \
    translations/ukui-power-manager-daemon_zh_CN.ts \
    translations/ukui-power-manager-daemon_bo_CN.ts \
    translations/ukui-power-manager-daemon_mn.ts \
    translations/ukui-power-manager-daemon_ug.ts \
    translations/ukui-power-manager-daemon_zh_HK.ts \
    translations/ukui-power-manager-daemon_kk.ts \
    translations/ukui-power-manager-daemon_ms.ts \
    translations/ukui-power-manager-daemon_vi.ts

QM_FILES_INSTALL_PATH = /usr/share/ukui-power-manager/daemon/translations/

# CONFIG += lrelase not work for qt5.6, add those from lrelease.prf for compatibility
qtPrepareTool(QMAKE_LRELEASE, lrelease)
lrelease.name = lrelease
lrelease.input = TRANSLATIONS
lrelease.output = ${QMAKE_FILE_IN_BASE}.qm
lrelease.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_OUT}
lrelease.CONFIG = no_link
QMAKE_EXTRA_COMPILERS += lrelease
PRE_TARGETDEPS += compiler_lrelease_make_all

for (translation, TRANSLATIONS) {
    translation = $$basename(translation)
    QM_FILES += $$OUT_PWD/$$replace(translation, \\..*$, .qm)
}
qm_files.files = $$QM_FILES
qm_files.path = $$QM_FILES_INSTALL_PATH
qm_files.CONFIG = no_check_exist
INSTALLS += qm_files

DEFINES += QM_FILES_INSTALL_PATH='\\"$${QM_FILES_INSTALL_PATH}\\"'

SOURCES += \
    acwatcher/acwatcher.cpp \
    cloudplatform.cpp \
    eventwatcher.cpp \
    globalconfig.cpp \
    gsettingwatcher/gsettingwatcher.cpp \
    hardware/huawei/flemingx.cpp \
    lowpowerwatcher/lowpowerwatcher.cpp \
    main.cpp \
    idle/idlenesswatcher.cpp \
    lidwatcher/lidwatcher.cpp \
    powermanagementdaemon.cpp \
    ukui-power-test-tool/upower-dbus/upm_upowerdbus.cpp \
    ukui-power-test-tool/upower-dbus/upowerbatterydbus.cpp \
    ukui-power-test-tool/upower-dbus/upowerdbus.cpp \
    ukui-power-test-tool/upower-dbus/upowerlinepowerdbus.cpp \
    ukui-power-test-tool/uptt-gsettings/upm_gsettings.cpp \
    ukui-power-test-tool/upttbatterydate.cpp \
    ukui-power-test-tool/upttbrightnessconfig.cpp \
    ukui-power-test-tool/upttcpupolicyconfig.cpp \
    ukui-power-test-tool/upttruntestbutton.cpp \
    ukui-power-test-tool/uptttestconfigpage.cpp \
    upmcustomdefaultgsettings.cpp \
    upmexternalinterface.cpp


HEADERS += \
    acwatcher/acwatcher.h \
    cloudplatform.h \
    eventwatcher.h \
    globalconfig.h \
    gsettingwatcher/gsettingwatcher.h \
    hardware/huawei/flemingx.h \
    lowpowerwatcher/lowpowerwatcher.h \
    idle/idlenesswatcher.h \
    lidwatcher/lidwatcher.h \
    common.h \
    powermanagementdaemon.h \
    ukui-power-test-tool/upower-dbus/upm_upowerdbus.h \
    ukui-power-test-tool/upower-dbus/upowerbatterydbus.h \
    ukui-power-test-tool/upower-dbus/upowerdbus.h \
    ukui-power-test-tool/upower-dbus/upowerlinepowerdbus.h \
    ukui-power-test-tool/uptt-gsettings/upm_gsettings.h \
    ukui-power-test-tool/upttbatterydate.h \
    ukui-power-test-tool/upttbrightnessconfig.h \
    ukui-power-test-tool/upttcpupolicyconfig.h \
    ukui-power-test-tool/upttruntestbutton.h \
    ukui-power-test-tool/uptttestconfigpage.h \
    upmcustomdefaultgsettings.h \
    upmexternalinterface.h

# Default rules for deployment.
qnx: target.path = /tmp/usr/bin
else: unix:!android: target.path = /usr/bin
!isEmpty(target.path): INSTALLS += target

desktop.files += resources/ukui-powermanagement-daemon.desktop
desktop.path = /etc/xdg/autostart/

upmGsettingsGlobalConf.files += resources/upm-gsettings-global.conf
upmGsettingsGlobalConf.path = /usr/share/ukui/ukui-power-manager/

upmGsettingsExtendConf.files += resources/extend-config/Inspur.config
upmGsettingsExtendConf.files += resources/extend-config/GreatWall.config
upmGsettingsExtendConf.files += resources/extend-config/suma.config
upmGsettingsExtendConf.files += resources/extend-config/Haier.config
upmGsettingsExtendConf.files += resources/extend-config/Lenovo.config
upmGsettingsExtendConf.files += resources/extend-config/KaiTian.config
upmGsettingsExtendConf.path = /usr/share/ukui/ukui-power-manager/upm-gsettings-extend.d/

INSTALLS += desktop upmGsettingsGlobalConf upmGsettingsExtendConf



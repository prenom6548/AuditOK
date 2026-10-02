#-------------------------------------------------
#
# Project created by QtCreator 2020-01-06T10:55:07
#
#-------------------------------------------------

QT       += core gui dbus KWindowSystem x11extras

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ukui-power-manager-tray
TEMPLATE = app

target.path = /usr/bin
INSTALLS += target

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
TRANSLATIONS+=\
    translations/ukui-power-manager-tray_ky.ts \
    translations/ukui-power-manager-tray_th.ts \
    translations/ukui-power-manager-tray_zh_CN.ts \
    translations/ukui-power-manager-tray_bo_CN.ts \
    translations/ukui-power-manager-tray_mn.ts \
    translations/ukui-power-manager-tray_ug.ts \
    translations/ukui-power-manager-tray_zh_HK.ts \
    translations/ukui-power-manager-tray_kk.ts \
    translations/ukui-power-manager-tray_ms.ts \
    translations/ukui-power-manager-tray_vi.ts

QM_FILES_INSTALL_PATH = /usr/share/ukui-power-manager/tray/translations/

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

# So we can access it from main.cpp
DEFINES += QM_FILES_INSTALL_PATH='\\"$${QM_FILES_INSTALL_PATH}\\"'

LIBS += -lukui-log4qt

CONFIG += c++11 no_keywords link_pkgconfig
PKGCONFIG +=  gsettings-qt x11 kysdk-qtwidgets  kysdk-waylandhelper kysdk-ukuiwindowhelper

INCLUDEPATH += $$PWD/../shared/include

SOURCES += \
    batteryiconhelper.cpp \
    divider.cpp \
    main.cpp \
    powertray.cpp \
    powerwindow.cpp \
    kylable.cpp \
    upm-gsettings/upm_gsettings.cpp \
    upower-dbus/upm_upowerdbus.cpp \
    upower-dbus/upowerbatterydbus.cpp \
    upower-dbus/upowerdbus.cpp \
    upower-dbus/upowerlinepowerdbus.cpp \
    xatom-helper.cpp \
    customstyle.cpp


HEADERS += \
    batteryiconhelper.h \
    divider.h \
    powertray.h \
    powerwindow.h \
    kylable.h \
    upm-gsettings/upm_gsettings.h \
    upower-dbus/upm_upowerdbus.h \
    upower-dbus/upowerbatterydbus.h \
    upower-dbus/upowerdbus.h \
    upower-dbus/upowerlinepowerdbus.h \
    xatom-helper.h \
    customstyle.h

FORMS +=
desktop.files += resources/ukui-power-manager-tray.desktop

desktop.path = /etc/xdg/autostart/

INSTALLS += desktop


# Default rules for deployment.


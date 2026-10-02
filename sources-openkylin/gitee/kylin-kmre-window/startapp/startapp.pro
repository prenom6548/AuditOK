# Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
#
# Authors:
#  Kobe Lee    lixiang@kylinos.cn
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 3.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.


QT       += core gui dbus xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets x11extras gui-private

TARGET = startapp
TEMPLATE = app

!system($$PWD/translations/generate_translations_pm.sh): error("Failed to generate pm")
qm_files.files = translations/*.qm
qm_files.path = /usr/share/startapp/translations/

inst1.files += startapp.desktop
inst1.path = /etc/xdg/autostart/
target.source  += $$TARGET
target.path = /usr/bin

appjson.files += ../data/app.json
appjson.path = /usr/share/kmre/

INSTALLS += target \
    appjson \
    inst1 \
    qm_files


# is V10 Professional or not
isEmpty(lsb_release):lsb_release=lsb_release
DISTRIB_DESCRIPTION = $$system($$lsb_release -d)
V10_PRO = $$system(cat /etc/lsb-release | grep -ci -e "Kylin\ V10\ Professional" -e "Kylin\ V10\ SP1" -e "Kylin\ V10\ SP2" -e "openKylin")
isEqual(V10_PRO, 0) {
    DEFINES += KYLIN_V10
} else {

}

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += link_pkgconfig

#QMAKE_LFLAGS += -ldl
LIBS += -ldl -lX11 -lpciaccess

SOURCES += \
    main.cpp \
    dialog.cpp \
    dbus_client.cpp \
    preferences.cpp \
    start_control.cpp \
    utils.cpp \
    worker.cpp \
    get_userinfo.cpp \
    containerthread.cpp \
    preinstallappmanager.cpp

HEADERS += \
    dialog.h \
    dbus_client.h \
    preferences.h \
    start_control.h \
    displayinfo.h \
    utils.h \
    worker.h \
    get_userinfo.h \
    containerthread.h \
    preinstallappmanager.h

FORMS += \
    dialog.ui

RESOURCES += \
    qsrc.qrc

# for wayland
# libkf5wayland-dev libwayland-dev
#DEFINES += UKUI_WAYLAND
#contains(DEFINES, UKUI_WAYLAND) {
#    QT += KWaylandClient
#    PKGCONFIG += wayland-client
#    SOURCES += ukui-wayland/ukui-decoration-manager.cpp ukui-wayland/ukui-decoration-core.c
#    HEADERS += ukui-wayland/ukui-decoration-manager.h ukui-wayland/ukui-decoration-client.h
#}

unix {
    UI_DIR = $$PWD/.ui
    MOC_DIR = $$PWD/.moc
    OBJECTS_DIR = $$PWD/.obj
    RCC_DIR = $$PWD/.rcc
}

TRANSLATIONS += \
    translations/startapp_zh_CN.ts \
    translations/startapp_bo_CN.ts

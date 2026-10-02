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

isEqual(QT_MAJOR_VERSION, 5) {
    QT += widgets gui core x11extras gui-private concurrent multimedia xml
}

unix {
    QT += dbus
    LIBS += $${QMAKE_LIBS_X11}
}

TARGET = kylin-kmre-settings
TEMPLATE = app
LANGUAGE = C++

CONFIG += c++17
CONFIG += qt warn_on
CONFIG += release
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt

LIBS += -ldl

DEFINES += SINGLE_INSTANCE

!system($$PWD/translations/generate_translations_pm.sh): error("Failed to generate pm")
qm_files.files = translations/*.qm
qm_files.path = /usr/share/kylin-kmre-settings/translations/
rm_kmre.files += ../data/rm_kmre.sh
rm_kmre.path = /usr/share/kmre/
target.source  += $$TARGET
target.path = /usr/bin
INSTALLS += qm_files \
    rm_kmre \
    target

QMAKE_CPPFLAGS *= $(shell dpkg-buildflags --get CPPFLAGS)
QMAKE_CFLAGS   *= $(shell dpkg-buildflags --get CFLAGS)
QMAKE_CXXFLAGS *= $(shell dpkg-buildflags --get CXXFLAGS)
QMAKE_LFLAGS   *= $(shell dpkg-buildflags --get LDFLAGS)

HEADERS += settingsframe.h \
    dbusclient.h \
    developerwidget.h \
    gamelistview.h \
    generalsettingwidget.h \
    global.h \
    phoneinfowidget.h \
    popuptip.h \
    settingscrollcontent.h \
    displaymodewidget.h \
    dockeripwidget.h \
    glesversionwidget.h \
    gamewidget.h \
    settingsitem.h \
    settingsgroup.h \
    inputitem.h \
    ipitem.h \
    messagebox.h \
    netmaskitem.h \
    navgationbutton.h \
    addgamewidget.h \
    removegamewidget.h \
    gamelistitem.h \
    radiobuttonitem.h \
    camerawidget.h \
    cleanerwidget.h \
    cleaneritem.h \
    logwidget.h \
    appmultiplierwidget.h \
    switchbutton.h \
    switchwidget.h \
    preferences.h \
    utils.h \
    processresult.h \
    xatom-helper.h \
    appwidget.h \
    appview.h \
    appitem.h \
    appsettingspanel.h \
    advancedwidget.h \
    getkmreinterface.h \
    appconfiguration.h \
    customwidget.h

SOURCES	+= main.cpp \
    dbusclient.cpp \
    developerwidget.cpp \
    gamelistview.cpp \
    generalsettingwidget.cpp \
    global.cpp \
    phoneinfowidget.cpp \
    popuptip.cpp \
    settingsframe.cpp \
    settingscrollcontent.cpp \
    displaymodewidget.cpp \
    dockeripwidget.cpp \
    glesversionwidget.cpp \
    gamewidget.cpp \
    settingsitem.cpp \
    settingsgroup.cpp \
    inputitem.cpp \
    ipitem.cpp \
    messagebox.cpp \
    netmaskitem.cpp \
    navgationbutton.cpp \
    addgamewidget.cpp \
    removegamewidget.cpp \
    gamelistitem.cpp \
    radiobuttonitem.cpp \
    camerawidget.cpp \
    cleanerwidget.cpp \
    cleaneritem.cpp \
    logwidget.cpp \
    appmultiplierwidget.cpp \
    switchbutton.cpp \
    switchwidget.cpp \
    preferences.cpp \
    utils.cpp \
    xatom-helper.cpp \
    appwidget.cpp \
    appview.cpp \
    appitem.cpp \
    appsettingspanel.cpp \
    advancedwidget.cpp \
    appconfiguration.cpp 

# qtsingleapplication
contains( DEFINES, SINGLE_INSTANCE ) {
    INCLUDEPATH += qtsingleapplication
    DEPENDPATH += qtsingleapplication
    SOURCES += qtsingleapplication/qtsingleapplication.cpp qtsingleapplication/qtlocalpeer.cpp
    HEADERS += qtsingleapplication/qtsingleapplication.h qtsingleapplication/qtlocalpeer.h
}

# for wayland
# libkf5wayland-dev libwayland-dev
#DEFINES += UKUI_WAYLAND
contains(DEFINES, UKUI_WAYLAND) {
    QT += KWaylandClient
    PKGCONFIG += wayland-client
    SOURCES += ukui-wayland/ukui-decoration-manager.cpp ukui-wayland/ukui-decoration-core.c ukui-wayland/plasma-shell-manager.cpp dialog.cpp
    HEADERS += ukui-wayland/ukui-decoration-manager.h ukui-wayland/ukui-decoration-client.h ukui-wayland/plasma-shell-manager.h dialog.h
}

RESOURCES += \
    res.qrc

unix {
    MOC_DIR = $$PWD/.moc
    OBJECTS_DIR = $$PWD/.obj
    RCC_DIR = $$PWD/.rcc
}

TRANSLATIONS += \
    translations/kylin-kmre-settings_zh_CN.ts \
    translations/kylin-kmre-settings_bo_CN.ts

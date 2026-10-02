# Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
#
# Authors:
#  Yuan ShanShan   yuanshanshan@kylinos.cn
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

TARGET = kylin-kmre-gps
QT += gui dbus core network webenginewidgets
CONFIG += c++11
CONFIG -= app_bundle
QT += widgets
DEFINES += SINGLE_INSTANCE

target.source += $$TARGET
target.path = /usr/bin

SOURCES += \
    currentlocationview.cpp \
    currentlocation.cpp \
    formatconversion.cpp \
    gpswindow.cpp \
    locationget.cpp \
    locationlistitem.cpp \
    locationlistwidget.cpp \
    searchbox.cpp \
    searchview.cpp \
    dbusclient.cpp \
    main.cpp

HEADERS += \
    currentlocationview.h \
    currentlocation.h \
    formatconversion.h \
    gpsglobal.h \
    gpswindow.h \
    locationget.h \
    locationlistitem.h \
    locationlistwidget.h \
    dbusclient.h \
    searchbox.h \
    searchview.h

RESOURCES += \
    res.qrc

# qtsingleapplication
contains( DEFINES, SINGLE_INSTANCE ) {
    INCLUDEPATH += qtsingleapplication
    DEPENDPATH += qtsingleapplication
    SOURCES += qtsingleapplication/qtsingleapplication.cpp qtsingleapplication/qtlocalpeer.cpp
    HEADERS += qtsingleapplication/qtsingleapplication.h qtsingleapplication/qtlocalpeer.h
}

# 配置gsettings
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt
settings.files += \
    $$PWD/data/org.kylin.gps.gschema.xml
settings.path = /usr/share/glib-2.0/schemas/

gps.files +=  \
    $$PWD/data/gps.conf
gps.path = /usr/share/kmre/

TRANSLATIONS += \
    $$PWD/translations/kylin-kmre-gps_zh_CN.ts \
    $$PWD/translations/kylin-kmre-gps_bo_CN.ts

!system($$PWD/translations/generate_translations_pm.sh): error("Failed to generate pm")
qm_files.files = translations/*.qm
qm_files.path = /usr/share/kylin-kmre-gps/translations/

INSTALLS +=settings \
    gps \
    qm_files \
    target

DISTFILES += \
    data/org.kylin.gps.gschema.xml \
    data/gps.conf

unix {
    UI_DIR = $$PWD/.ui
    MOC_DIR = $$PWD/.moc
    OBJECTS_DIR = $$PWD/.obj
    RCC_DIR = $$PWD/.rcc
}
#  -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
#
#  Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
QT += core gui widgets dbus KScreen KWaylandClient# multimedia multimediawidgets

TEMPLATE = lib
DEFINES += TOUCHSCREENSETTINGS_LIBRARY
CONFIG += plugin

CONFIG += c++11 \
          link_pkgconfig

PKGCONFIG += gsettings-qt \
             kysdk-qtwidgets

TARGET = $$qtLibraryTarget(touchscreenui)
target.path = $$[QT_INSTALL_LIBS]/ukui-control-center

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

#QMAKE_LN_SHLIB = :

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

LIBS += -lukcc \
        -lxcb-xinput \
        -lxcb \
        -lXi

include(../gesture-widget/gesture-widget.pri)
include(item-widget/item-widget.pri)
include(flow-layout/flow-layout.pri)
include(../native-filter/native-filter.pri)
include(../settings-common/settings-common.pri)
#include(video-widget/video-widget.pri)
include(calibration/calibration.pri)

SOURCES += \
    autotabletmodewidget.cpp \
    drawiconlabel.cpp \
    gestureguidance.cpp \
    picturebutton.cpp \
    pictureiconlabel.cpp \
    tabletmodedesktopwidget.cpp \
    tabletmodewidget.cpp \
    touchscreen-settings.cpp \
    touchscreen.cpp

HEADERS += \
    autotabletmodewidget.h \
    drawiconlabel.h \
    gestureguidance.h \
    picturebutton.h \
    pictureiconlabel.h \
    tabletmodedesktopwidget.h \
    tabletmodewidget.h \
    touchscreen-settings.h \
    touchscreen.h

TRANSLATIONS += \
    translations/zh_CN.ts \
    translations/bo_CN.ts \
    translations/mn_MN.ts \
    translations/zh_HK.ts \
    translations/kk_KZ.ts \
    translations/ug_CN.ts \
    translations/ky_KG.ts \
    translations/de_DE.ts \
    translations/es_ES.ts \
    translations/fr_FR.ts

isEmpty(PREFIX) {
    PREFIX = /usr
}

qm_files.path = $${PREFIX}/share/touchscreen-settings/translations
qm_files.files = translations/*

CONFIG(release, debug|release) {
    !system($$PWD/translate_generation.sh): error("Failed to generate translation")
}

INSTALLS += target \
            qm_files

FORMS += \
    gestureguidance.ui \
    touchscreen.ui

RESOURCES += \
    resources.qrc

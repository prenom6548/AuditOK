QT += core gui widgets dbus# multimedia multimediawidgets

TEMPLATE = lib
DEFINES += STYLUSSETTINGS_LIBRARY
CONFIG += plugin

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


CONFIG += c++11 \
          link_pkgconfig

PKGCONFIG += gsettings-qt

TARGET = $$qtLibraryTarget(stylussetting)
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

LIBS += \
    -lukcc

isEmpty(PREFIX) {
    PREFIX = /usr
}

HEADERS += \
    stylus-settings.h \
    stylussettingpage.h

SOURCES += \
    stylus-settings.cpp \
    stylussettingpage.cpp

FORMS += \
    stylussettingpage.ui

RESOURCES += \
    resource.qrc

TRANSLATIONS += \
    translations/zh_CN.ts \
    translations/mn_MN.ts \
    translations/kk_KZ.ts \
    translations/ug_CN.ts \
    translations/ky_KG.ts \

qm_files.path = $${PREFIX}/share/stylus-settings/translations
qm_files.files = translations/*

CONFIG(release, debug|release) {
    !system($$PWD/translate_generation.sh): error("Failed to generate translation")
}

INSTALLS += target \
            qm_files

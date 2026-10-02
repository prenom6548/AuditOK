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

QT += core network
QT -= gui

CONFIG += console
CONFIG -= app_bundle

QT += dbus
QT += xml

TEMPLATE = app
TARGET = kylin-kmre-settings-daemon

target.source += $$TARGET
target.path = /usr/bin

services.files += cn.kylinos.Kmre.Pref.service
services.path = /usr/share/dbus-1/system-services/

conf.files += cn.kylinos.Kmre.Pref.conf
conf.path = /etc/dbus-1/system.d/

INSTALLS += target \
    services \
    conf 

SOURCES += main.cpp \
    prefmanager.cpp \

HEADERS += \
    prefmanager.h \

unix {
    MOC_DIR = $$PWD/.moc
    OBJECTS_DIR = $$PWD/.obj
}

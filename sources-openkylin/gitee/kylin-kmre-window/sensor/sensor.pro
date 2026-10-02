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

#LIBS += -lasan
TARGET = kylin-kmre-sensorwindow
QT += gui dbus core network
CONFIG += c++11
CONFIG -= app_bundle
QT += widgets

DEFINES += SINGLE_INSTANCE
LIBS += -lX11
target.source += $$TARGET
target.path = /usr/bin

# qtsingleapplication
contains( DEFINES, SINGLE_INSTANCE ) {
    INCLUDEPATH += qtsingleapplication
    DEPENDPATH += qtsingleapplication
    SOURCES += qtsingleapplication/qtsingleapplication.cpp qtsingleapplication/qtlocalpeer.cpp
    HEADERS += qtsingleapplication/qtsingleapplication.h qtsingleapplication/qtlocalpeer.h
}

SOURCES += main.cpp \
    directbtn.cpp \
    sensorwindow.cpp

HEADERS += \
    directbtn.h \
    sensorwindow.h

INSTALLS +=target
#QMAKE_CXXFLAGS += -fsanitize=address -fsanitize-recover=all -fsanitize=leak -fno-omit-frame-pointer -g

unix {
    UI_DIR = $$PWD/.ui
    MOC_DIR = $$PWD/.moc
    OBJECTS_DIR = $$PWD/.obj
    RCC_DIR = $$PWD/.rcc
}
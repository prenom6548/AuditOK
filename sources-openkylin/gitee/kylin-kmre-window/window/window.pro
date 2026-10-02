# Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
#
# Authors:
#  Alan Xie    xiehuijun@kylinos.cn
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

QT += dbus network concurrent
isEqual(QT_MAJOR_VERSION, 5) {
    QT += widgets gui core x11extras gui-private multimedia xml egl_support-private
}

QT += KWaylandClient
greaterThan(QT_MAJOR_VERSION, 5) {
    QT += waylandclient-private
}
equals(QT_MAJOR_VERSION, 5) {
    greaterThan(QT_MINOR_VERSION, 14) {
        QT += waylandclient-private
    }
}

TARGET = kylin-kmre-window
TEMPLATE = app

!system($$PWD/translations/generate_translations_pm.sh): error("Failed to generate pm")
qm_files.files = translations/*.qm
qm_files.path = /usr/share/kylin-kmre-window/translations/

target.source  += $$TARGET
target.path = /usr/bin

kmre_base_config.files += ../data/games.json
kmre_base_config.files += ../data/warning.json
kmre_base_config.files += ../data/feature_config.ini
kmre_base_config.path = /usr/share/kmre/

dbus_service.files += ../data/cn.kylinos.Kmre.Window.service
dbus_service.path = /usr/share/dbus-1/services/

kmre_config.files += ../data/dynamic_size_app_list.xml
kmre_config.files += ../data/kmre_data_app_list.xml
kmre_config.path = /usr/share/kmre/config/

INSTALLS += target \
    kmre_base_config \
    dbus_service \
    qm_files \
    kmre_config

# is V10 Professional or not
isEmpty(lsb_release):lsb_release=lsb_release
DISTRIB_DESCRIPTION = $$system($$lsb_release -d)
V10_PRO = $$system(cat /etc/lsb-release | grep -ci -e "Kylin\ V10\ Professional" -e "Kylin\ V10\ SP1" -e "Kylin\ V10\ SP2" -e "openKylin" -e "Ubuntu")
isEqual(V10_PRO, 0) {
    DEFINES += KYLIN_V10
    guide.files += ../data/v10/kmre/
    guide.path = /usr/share/kylin-user-guide/data/guide/

    INSTALLS += guide
} 
else {
    guide.files += ../data/v10pro/kmre/
    guide.path = /usr/share/kylin-user-guide/data/guide/

    INSTALLS += guide
}

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++17
CONFIG += qt warn_on
CONFIG += release
CONFIG += link_pkgconfig
PKGCONFIG += xcb xcb-randr xcb-icccm xcb-xinerama xcb-dri2 xcb-dri3 \
            gsettings-qt protobuf libdrm libavformat libavcodec libavutil libswscale \
            wayland-client

CONFIG -= qmake_use

QMAKE_CPPFLAGS *= $(shell dpkg-buildflags --get CPPFLAGS)
QMAKE_CFLAGS   *= $(shell dpkg-buildflags --get CFLAGS)
QMAKE_CXXFLAGS *= $(shell dpkg-buildflags --get CXXFLAGS)
QMAKE_LFLAGS   *= $(shell dpkg-buildflags --get LDFLAGS)
#QMAKE_CFLAGS_DEBUG += -g
QMAKE_CXXFLAGS *= -fpermissive

DEFINES += SDL_SUPPORTED

LIBS += -ldl -lX11 -lXtst -lpthread -lboost_system -lboost_filesystem -lDisplayControl \
        -lGLESv2 -lEGL -rdynamic -lasound -lSDL2 -lwayland-client -lepoxy -lsystemd \
        -lXfixes -lXinerama -lXext

INCLUDEPATH += /usr/include/DisplayControl/ /usr/include/boost/ ./dbus/ \
            ./common/ ./communication/ ./config/ ./window/ ./gamekey/ ./utilitys/ \
            ./display/ ./display/displaybackend/ ./display/displayfrontend/ ./display/displaymanager/ \
            ./event/ ./record/ ./screensharing/ ./wayland/ukui/ ./wayland/drm/ \
            ./wayland/xdg/ ./wayland/gtk/

!system($$PWD/gen_proto_and_qdbus_src.sh): error("Failed to generate protobuf or qdbus source!")
system(wayland-scanner client-header wayland/remote-access.xml wayland/remote-access.h)
system(wayland-scanner private-code wayland/remote-access.xml wayland/remote-access.c)
system(wayland-scanner client-header wayland/xdg-output-unstable-v1.xml wayland/xdg-output-unstable-v1.h)
system(wayland-scanner private-code wayland/xdg-output-unstable-v1.xml wayland/xdg-output-unstable-v1.c)

SOURCES += main.cpp

SourceDir = ./display \
            ./common \
            ./config \
            ./window \
            ./dbus \
            ./utilitys \
            ./communication \
            ./screensharing \
            ./gamekey \
            ./event \
            ./record \
            ./wayland

for(var, SourceDir) {
    SOURCES += $$files($$join(var, , , /*.c), true)
    SOURCES += $$files($$join(var, , , /*.cc), true)
    SOURCES += $$files($$join(var, , , /*.cpp), true)
    HEADERS += $$files($$join(var, , , /*.h), true)
}


#message("SOURCES: $$SOURCES")

SourceDir = .
for(var, SourceDir) {
    FORMS       += $$files($$join(var, , , /*.ui) , true)
    RESOURCES   += $$files($$join(var, , , /*.qrc), true)
}

unix {
    UI_DIR = $$PWD/.ui
    MOC_DIR = $$PWD/.moc
    OBJECTS_DIR = $$PWD/.obj
    RCC_DIR = $$PWD/.rcc
}

TRANSLATIONS += \
    translations/kylin-kmre-window_zh_CN.ts \
    translations/kylin-kmre-window_bo_CN.ts

DISTFILES += \
    ../data/feature_config.ini

#-------------------------------------------------
#
# Project created by QtCreator 2019-06-29T09:55:08
#
#-------------------------------------------------
include(../../../env.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/Label/label.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/Button/button.pri)
include($$PROJECT_ROOTDIR/libukcc/interface.pri)

QT       += widgets

TEMPLATE = lib
CONFIG += plugin

TARGET = $$qtLibraryTarget(backup)
DESTDIR = ../..
target.path = $${PLUGIN_INSTALL_DIRS}
INSTALLS += target

INCLUDEPATH   +=  \
                 $$PROJECT_ROOTDIR \

##加载gio库和gio-unix库
CONFIG        += link_pkgconfig \
                 C++11
PKGCONFIG     += gio-2.0 \
                 gio-unix-2.0 \
                 gsettings-qt \
                 kysdk-sysinfo \
                 kysdk-datacollect \



#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        backup.cpp

HEADERS += \
        backup.h

FORMS += \
        backup.ui

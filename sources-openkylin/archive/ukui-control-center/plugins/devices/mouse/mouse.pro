#-------------------------------------------------
#
# Project created by QtCreator 2019-08-22T11:12:59
#
#-------------------------------------------------

include(../../../env.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/Label/label.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/SettingWidget/settingwidget.pri)
include($$PROJECT_ROOTDIR/libukcc/interface.pri)

QT       += widgets x11extras dbus

#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TEMPLATE = lib
CONFIG += plugin

TARGET = $$qtLibraryTarget(mouse)
DESTDIR = ../..
target.path = $${PLUGIN_INSTALL_DIRS}

INCLUDEPATH   +=  \
                 $$PROJECT_ROOTDIR \

LIBS          +=  -L$$[QT_INSTALL_LIBS] -lgsettings-qt

##加载gio库和gio-unix库
CONFIG        += link_pkgconfig \
                 C++11
PKGCONFIG     += gio-2.0 \
                 gio-unix-2.0 \
                 gsettings-qt \
                 x11 \
                 kysdk-qtwidgets \
                 kysdk-datacollect \

#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        ../../personalized/theme/drawiconlabel.cpp \
        ../../personalized/theme/themebutton.cpp \
        ../../personalized/theme/themeiconlabel.cpp \
        mouse.cpp \
        mouseui.cpp

HEADERS += \
        ../../personalized/theme/drawiconlabel.h \
        ../../personalized/theme/themebutton.h \
        ../../personalized/theme/themeiconlabel.h \
        mouse.h \
        mouseui.h

FORMS +=

INSTALLS += target

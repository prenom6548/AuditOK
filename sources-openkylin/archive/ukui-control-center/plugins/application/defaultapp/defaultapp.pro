#-------------------------------------------------
#
# Project created by QtCreator 2019-02-20T15:36:43
#
#-------------------------------------------------

include(../../../env.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/Label/label.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/SettingWidget/settingwidget.pri)
include($$PROJECT_ROOTDIR/libukcc/interface.pri)

QT            += widgets dbus concurrent KCoreAddons KService
TEMPLATE = lib
CONFIG        += plugin

TARGET = $$qtLibraryTarget(defaultapp)
DESTDIR = ../..
target.path = $${PLUGIN_INSTALL_DIRS}
INSTALLS += target


##加载gio库和gio-unix库，用于处理desktop文件
CONFIG        += link_pkgconfig \
                 C++11
PKGCONFIG     += gio-2.0 \
                 gio-unix-2.0 \
                 gsettings-qt \
                 kysdk-datacollect \

INCLUDEPATH   +=  \
                 $$PROJECT_ROOTDIR \

#LIBS          += -L$$[QT_INSTALL_LIBS] -ldefaultprograms \

SOURCES += \
    defaultapp.cpp \
#    component/custdomcombobox.cpp
    defaultappui.cpp

HEADERS += \
    defaultapp.h \
#    component/custdomcombobox.h
    defaultappui.h \
    service.h

FORMS +=

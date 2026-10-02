#-------------------------------------------------
#
# Project created by QtCreator 2019-06-17T11:16:02
#
#-------------------------------------------------
include(../../../env.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/AddBtn/addbtn.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/Label/label.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/MaskWidget/maskwidget.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/FlowLayout/flowlayout.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/HoverWidget/hoverwidget.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/SettingWidget/settingwidget.pri)
include($$PROJECT_ROOTDIR/libukcc/interface.pri)

QT       += widgets xml dbus concurrent KService KCoreAddons KConfigCore

TEMPLATE = lib
CONFIG += plugin

TARGET = $$qtLibraryTarget(wallpaper)
DESTDIR = ../..
target.path = $${PLUGIN_INSTALL_DIRS}
INSTALLS += target

INCLUDEPATH   +=  \
                 $$PROJECT_ROOTDIR \

LIBS += -L$$[QT_INSTALL_LIBS] -lgsettings-qt

##加载gio库和gio-unix库，用于获取和设置enum类型的gsettings
CONFIG        += link_pkgconfig \
                 C++11
PKGCONFIG     += gio-2.0 \
                 gio-unix-2.0 \
                 gsettings-qt \
                 kysdk-datacollect \

#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    colordialog.cpp \
    colorpreview.cpp \
    colorsliderstyle.cpp \
    colorsquare.cpp \
    gradientslider.cpp \
    pictureunit.cpp \
    previewlabel.cpp \
    wallpaper.cpp \
    wallpaperui.cpp \
    component/custdomitemmodel.cpp

HEADERS += \
    colordialog.h \
    colorpreview.h \
    colorsliderstyle.h \
    colorsquare.h \
    gradientslider.h \
    pictureunit.h \
    previewlabel.h \
    wallpaper.h \
    wallpaperui.h \
    component/custdomitemmodel.h


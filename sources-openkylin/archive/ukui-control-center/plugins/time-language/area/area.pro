#-------------------------------------------------
#
# Project created by QtCreator 2019-06-29T15:14:42
#
#-------------------------------------------------
include(../../../env.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/Label/label.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/AddBtn/addbtn.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/HoverWidget/hoverwidget.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/Frame/frame.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/SettingWidget/settingwidget.pri)
include($$PROJECT_ROOTDIR/libukcc/interface.pri)

QT       += widgets dbus KWidgetsAddons network

TEMPLATE = lib
CONFIG += plugin

TARGET = $$qtLibraryTarget(area)
DESTDIR = ../..
target.path = $${PLUGIN_INSTALL_DIRS}
INSTALLS += target

LIBS += -L$$[QT_INSTALL_LIBS]
INCLUDEPATH   += $$PROJECT_ROOTDIR

##加载gio库和gio-unix库，用于处理时间
CONFIG        += link_pkgconfig \
                 C++11
PKGCONFIG     += gio-2.0 \
                 gio-unix-2.0 \
                 kysdk-qtwidgets \
                 kysdk-datacollect \
                 kysdk-systime \
                 kysdk-package

#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        addinputmethoddialog.cpp \
        addlanguagedialog.cpp \
        area.cpp \
        areaui.cpp \
        installlanguagefeaturesdialog.cpp \
        languagerestartbar.cpp \
        languagefeatureconfig.cpp \
        languagefeatureitem.cpp \
        languageinstall.cpp \
        languageframe.cpp
#        languageinfo.cpp

HEADERS += \
        addinputmethoddialog.h \
        addlanguagedialog.h \
        area.h \
        areaui.h \
        installlanguagefeaturesdialog.h \
        languagerestartbar.h \
        languagefeatureconfig.h \
        languagefeatureitem.h \
        languageinstall.h \
        languageframe.h
#        languageinfo.h

FORMS += \
        addinputmethoddialog.ui

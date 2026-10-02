#-------------------------------------------------
#
# Project created by QtCreator 2019-08-22T11:12:59
#
#-------------------------------------------------

include(../../../env.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/Label/label.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/SettingWidget/settingwidget.pri)
include($$PROJECT_ROOTDIR/libukcc/interface.pri)

QT       += widgets quick
#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TEMPLATE = lib
CONFIG += plugin

TARGET = $$qtLibraryTarget(keyboard)
DESTDIR = ../..
target.path = $${PLUGIN_INSTALL_DIRS}

INCLUDEPATH   +=  \
                 $$PROJECT_ROOTDIR \

LIBS          +=  -L$$[QT_INSTALL_LIBS] -lgsettings-qt -lukui-quick-platform

CONFIG        += link_pkgconfig \
                 C++11
PKGCONFIG     += gsettings-qt \
                 kysdk-qtwidgets \
                 kysdk-datacollect \
                 ukui-quick-platform \
                 ukui-quick-framework \
                 ukui-quick-core

#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    configgroup.cpp \
    configitem.cpp \
    configmanager.cpp \
    listoptionitem.cpp \
    keyboardmain.cpp \
    keyboardui.cpp \
    fcitxdbustypes.cpp \
    fcitxdbusprovider.cpp \
    inputmethodlistmodel.cpp \
    kglobalacceldbusproxy.cpp \
    kglobaldbustypes.cpp \
    languagecode.cpp \
    layoutmanager.cpp \
    layoutmodel.cpp \
    menumanager.cpp \
    searchinputmethodmodel.cpp \
    inputmethodconfigtoolview.cpp \
    inputmethodconfigtool.cpp \
    shortcutcontroller.cpp

HEADERS += \
    configgroup.h \
    configitem.h \
    configmanager.h \
    configvarianthelper.h \
    listoptionitem.h \
    keyboardmain.h \
    keyboardui.h \
    fcitxdbustypes.h \
    fcitxdbusprovider.h \
    fcitxcontrollerproxy.h \
    inputmethodlistmodel.h \
    kglobalacceldbusproxy.h \
    kglobaldbustypes.h \
    languagecode.h \
    layoutmanager.h \
    layoutmodel.h \
    menumanager.h \
    searchinputmethodmodel.h \
    inputmethodconfigtoolview.h \
    inputmethodconfigtool.h \
    shortcutcontroller.h


INSTALLS += target

RESOURCES += \
    qml.qrc

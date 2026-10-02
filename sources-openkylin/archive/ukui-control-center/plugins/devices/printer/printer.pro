#-------------------------------------------------
#
# Project created by QtCreator 2019-02-28T14:09:42
#
#-------------------------------------------------

include(../../../env.pri)

include($$PROJECT_ROOTDIR/libukcc/widgets/Label/label.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/AddBtn/addbtn.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/HoverWidget/hoverwidget.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/SettingWidget/settingwidget.pri)
include($$PROJECT_ROOTDIR/libukcc/interface.pri)

QT       += widgets printsupport dbus

LIBS          += -L$$[QT_INSTALL_LIBS] -lcups

TEMPLATE = lib
CONFIG += plugin \
          link_pkgconfig

PKGCONFIG += gsettings-qt \
             kysdk-datacollect \

TARGET = $$qtLibraryTarget(printer)
DESTDIR = ../..
target.path = $${PLUGIN_INSTALL_DIRS}

INCLUDEPATH += \
               $$PROJECT_ROOTDIR \

#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        printer.cpp \
        printerbtn.cpp \
        usbthread.cpp

HEADERS += \
        printer.h \
        printerbtn.h \
        usbthread.h

FORMS +=

INSTALLS += target

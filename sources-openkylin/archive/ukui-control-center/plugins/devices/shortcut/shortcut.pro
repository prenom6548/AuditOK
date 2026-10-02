include(../../../env.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/HoverWidget/hoverwidget.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/Label/label.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/AddBtn/addbtn.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/Frame/frame.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/SettingWidget/settingwidget.pri)
include($$PROJECT_ROOTDIR/libukcc/interface.pri)

QT       += widgets dbus KXmlGui KGlobalAccel
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = lib
CONFIG += plugin

TARGET = $$qtLibraryTarget(shortcut)
DESTDIR = ../..
target.path = $${PLUGIN_INSTALL_DIRS}

INCLUDEPATH += \
               $$PROJECT_ROOTDIR \
               /usr/include/dconf \

LIBS        += -L$$[QT_INSTALL_LIBS] -lgsettings-qt -ldconf

CONFIG += link_pkgconfig c++11

PKGCONFIG += gio-2.0 \
             gio-unix-2.0 \
             gsettings-qt \
             kysdk-datacollect \

DEFINES += QT_DEPRECATED_WARNINGS

#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    addshortcutdialog.cpp \
    doubleclicklineedit.cpp \
    clickfixlabel.cpp \
    realizeshortcutwheel.cpp \
    shortcut.cpp \
    shortcutline.cpp \
    shortcutui.cpp

HEADERS += \
    clickfixlabel.h \
    addshortcutdialog.h \
    doubleclicklineedit.h \
    realizeshortcutwheel.h \
    shortcut.h \
    shortcutline.h \
    shortcutui.h

INSTALLS += target

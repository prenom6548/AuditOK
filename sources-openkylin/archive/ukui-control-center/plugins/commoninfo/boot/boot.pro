include(../../../env.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/Label/label.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/SettingWidget/settingwidget.pri)
include($$PROJECT_ROOTDIR/libukcc/interface.pri)

QT       += core gui dbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

LIBS          += -L$$[QT_INSTALL_LIBS] -lpolkit-qt5-core-1

TEMPLATE = lib
CONFIG += plugin \
          link_pkgconfig

PKGCONFIG     += gsettings-qt \
                 kysdk-qtwidgets \
                 kysdk-sysinfo \
                 kysdk-waylandhelper

TARGET = $$qtLibraryTarget(boot)
DESTDIR = ../..
target.path = $${PLUGIN_INSTALL_DIRS}

INCLUDEPATH   +=  \
                 $$PROJECT_ROOTDIR \

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    boot.cpp \
    grubverifydialog.cpp \
    loadingwidget.cpp \

HEADERS += \
    boot.h \
    grubverifydialog.h \
    loadingwidget.h \

# Default rules for deployment.
INSTALLS += target


QT       += widgets dbus x11extras svg

TEMPLATE = lib
CONFIG += plugin \
          link_pkgconfig \
          C++11

include(../../env.pri)
include(../../CommonControl/CommonControl.pri)
include(../../dataworker/dataworker.pri)

TARGET = $$qtLibraryTarget(machineinfo)
DESTDIR = ..
target.path = $${PLUGIN_INSTALL_DIRS}
INSTALLS += target

LIBS += -L$$[QT_INSTALL_LIBS] -lgsettings-qt

PKGCONFIG     += gsettings-qt
PKGCONFIG     += kysdk-utils
PKGCONFIG     += kysdk-sysinfo

SOURCES += \
    infopage.cpp \
    machineinfo.cpp

HEADERS += \
    infopage.h \
    machineinfo.h

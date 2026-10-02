INCLUDEPATH += $$PWD $$PWD/interface
QT += dbus

CONFIG += link_pkgconfig

PKGCONFIG += kysdk-diagnostics

SOURCES += \
    $$PWD/interface/ukcccommon.cpp


HEADERS += \
        $$PWD/interface/interface.h \
    $$PWD/interface/ukcccommon.h

INCLUDEPATH += $$PWD $$PWD/interface
QT += dbus

CONFIG += link_pkgconfig

PKGCONFIG += kysdk-datacollect

SOURCES += \
        $$PWD/interface/mthread.cpp \
        $$PWD/interface/ukcccommon.cpp \
        $$PWD/interface/subplugininterface.cpp

HEADERS += \
        $$PWD/interface/interface.h \
        $$PWD/interface/subplugininterface.h \
        $$PWD/interface/subplugininterface_p.h \
        $$PWD/interface/mthread.h \
    $$PWD/interface/ukcccommon.h

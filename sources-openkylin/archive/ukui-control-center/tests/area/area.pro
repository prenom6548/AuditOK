QT += gui testlib dbus
CONFIG += qt warn_on depend_includepath testcase

CONFIG += c++11 console link_pkgconfig
PKGCONFIG += gsettings-qt

TEMPLATE = app

HEADERS += ../../registeredSession/area/areainterface.h \
           ../../registeredSession/universalinterface.h \
           areatest.h

SOURCES +=  \
            ../../registeredSession/area/areainterface.cpp \
            ../../registeredSession/universalinterface.cpp \
            areatest.cpp

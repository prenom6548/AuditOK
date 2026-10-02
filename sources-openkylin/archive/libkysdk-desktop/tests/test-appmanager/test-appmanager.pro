QT += testlib widgets gui dbus

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

CONFIG += link_pkgconfig
PKGCONFIG += glib-2.0

TEMPLATE = app

QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage -fno-inline -fno-access-control -fno-exceptions
QMAKE_CXXFLAGS += --coverage

SOURCES +=  \
        appmanagertest.cpp \
        ../../kysdk-appmanager/src/kappmanager.cpp \
        ../../kysdk-appmanager/src/appinfo.cpp

HEADERS += \
        ../../kysdk-appmanager/src/kappmanager.h \
        ../../kysdk-appmanager/src/appinfo.h

QT += testlib widgets gui dbus
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage -fno-inline -fno-access-control -fno-exceptions
QMAKE_CXXFLAGS += --coverage

SOURCES +=  \
    ../../kysdk-notification/src/knotifier.cpp \
    notificationtest.cpp

HEADERS += \
    ../../kysdk-notification/src/knotifier.h

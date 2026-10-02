QT += testlib gui widgets dbus multimedia
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt

QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage -fno-inline -fno-access-control -fno-exceptions
QMAKE_CXXFLAGS += --coverage

SOURCES +=  \
    ../../kysdk-soundeffects/src/ksoundeffects.cpp \
    soundeffectstest.cpp

HEADERS += \
    ../../kysdk-soundeffects/src/ksoundeffects.h \
    ../../kysdk-soundeffects/src/kysdk-soundeffects_global.h

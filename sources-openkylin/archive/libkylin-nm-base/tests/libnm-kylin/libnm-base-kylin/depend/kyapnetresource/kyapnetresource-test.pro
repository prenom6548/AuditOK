QT       += core testlib dbus network

QT       -= gui

CONFIG += c++14 qt link_pkgconfig console warn_on
CONFIG -= app_bundle

TARGET = test_kyapnetresource
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

PROJECT_ROOT = $$PWD/../../../../..
INCLUDEPATH += $$PROJECT_ROOT/libnm-kylin/libnm-base-kylin/depend
INCLUDEPATH += $$PROJECT_ROOT/libnm-kylin/libnm-base-kylin
INCLUDEPATH += $$PROJECT_ROOT/libnm-kylin
INCLUDEPATH += /usr/include/KF5/NetworkManagerQt
INCLUDEPATH += /usr/include/dbus-1.0/dbus
INCLUDEPATH += /usr/include/libnm
INCLUDEPATH += /usr/include/glib-2.0


LIBS += -L$$PROJECT_ROOT/libnm-kylin/libnm-base-kylin -lkylin-nm-base
LIBS += -L/usr/lib/ -lKF5NetworkManagerQt
LIBS += -lgcov

QMAKE_CXXFLAGS += --coverage
QMAKE_LFLAGS += --coverage

PKGCONFIG += libnm glib-2.0

SOURCES += test_kyapnetresource.cpp main.cpp

HEADERS += test_kyapnetresource.h

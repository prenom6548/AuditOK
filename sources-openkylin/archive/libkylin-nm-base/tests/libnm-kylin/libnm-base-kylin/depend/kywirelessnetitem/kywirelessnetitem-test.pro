QT       += core testlib dbus network

QT       -= gui

CONFIG += c++14 qt link_pkgconfig console warn_on
CONFIG += coverage
CONFIG -= app_bundle

TARGET = test_kywirelessnetitem
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += LIBNMBASEKYLIN_LIBRARY

QMAKE_CXXFLAGS += --coverage

INCLUDEPATH += ../../../../../libnm-kylin/libnm-base-kylin/depend
INCLUDEPATH += ../../../../../libnm-kylin/libnm-base-kylin
INCLUDEPATH += ../../../../../libnm-kylin
INCLUDEPATH += /usr/include/KF5/NetworkManagerQt
INCLUDEPATH += /usr/include/dbus-1.0/dbus


LIBS += -L../../../../../libnm-kylin/libnm-base-kylin -lkylin-nm-base
LIBS += -L/usr/lib/ -lKF5NetworkManagerQt
LIBS += --coverage
LIBS += -lgcov
LIBS += -lgtest
LIBS += -lpthread

PKGCONFIG += libnm glib-2.0

SOURCES += test_kywirelessnetitem.cpp main.cpp

HEADERS += test_kywirelessnetitem.h
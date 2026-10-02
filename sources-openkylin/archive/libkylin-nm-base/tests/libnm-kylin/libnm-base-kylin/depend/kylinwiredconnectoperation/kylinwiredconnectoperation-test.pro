QT       += core testlib dbus network

QT       -= gui

CONFIG += c++14 qt link_pkgconfig console warn_on
CONFIG -= app_bundle
CONFIG += --coverage

TARGET = test_kylinwiredconnectoperation
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += ../../../../../libnm-kylin/libnm-base-kylin/depend
INCLUDEPATH += ../../../../../libnm-kylin/libnm-base-kylin
INCLUDEPATH += /usr/include/KF5/NetworkManagerQt
INCLUDEPATH += /usr/include/dbus-1.0/dbus


LIBS += -L../../../../../libnm-kylin/libnm-base-kylin -lkylin-nm-base
LIBS += -L/usr/lib/ -lKF5NetworkManagerQt
LIBS += -lgcov

PKGCONFIG += libnm glib-2.0

SOURCES += test_kylinwiredconnectoperation.cpp \
            main.cpp

HEADERS += test_kylinwiredconnectoperation.h

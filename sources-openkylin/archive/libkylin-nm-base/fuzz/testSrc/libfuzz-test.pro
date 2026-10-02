QT       += core dbus network
QT       -= gui

TARGET = libfuzz-test
TEMPLATE = app
CONFIG += console c++11 warn_on link_pkgconfig
CONFIG -= app_bundle

DEFINES += FUZZ_TEST

PKGCONFIG += libnm gsettings-qt gtk+-3.0 

INCLUDEPATH += /usr/include/KF5/NetworkManagerQt
INCLUDEPATH += $$PWD/../../libnm-kylin/libnm-base-kylin
INCLUDEPATH += $$PWD/../../libnm-kylin/libnm-base-kylin/depend


LIBS    +=  -L/usr/lib/ -lKF5NetworkManagerQt
LIBS    +=  -L$$PWD/../../libnm-kylin/libnm-base-kylin -lkylin-nm-base


SOURCES += \
    fuzz_enterprise_setting.cpp \
    fuzz_kylin_networkmanager.cpp \
    fuzz_test_main.cpp

HEADERS +=

QT       += core testlib dbus network

QT       -= gui

CONFIG += c++14 qt link_pkgconfig console warn_on
CONFIG -= app_bundle

TARGET = test_kylinconnectsetting
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

#代码覆盖率工具gcov
QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage
QMAKE_CXXFLAGS += --coverage -fno-inline -fno-access-control -fno-exceptions


INCLUDEPATH += ../../../../../libnm-kylin/libnm-base-kylin/depend
INCLUDEPATH += ../../../../../libnm-kylin/libnm-base-kylin
INCLUDEPATH += ../../../../../libnm-kylin
INCLUDEPATH += /usr/include/KF5/NetworkManagerQt
INCLUDEPATH += /usr/include/dbus-1.0/dbus


LIBS += -L../../../../../libnm-kylin/libnm-base-kylin -lkylin-nm-base
LIBS += -L/usr/lib/ -lKF5NetworkManagerQt
LIBS += --coverage
LIBS += -lgcov


PKGCONFIG += \
    libnm \
    glib-2.0

SOURCES += \
    test_kylinconnectsetting.cpp \
    main.cpp

HEADERS += \
    test_kylinconnectsetting.h


QT       += core testlib dbus network

QT       -= gui

CONFIG += c++14 qt link_pkgconfig console warn_on
CONFIG -= app_bundle

TARGET = test_kylingeneral
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

#代码覆盖率工具gcov
QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage
QMAKE_CXXFLAGS += --coverage -fno-inline -fno-access-control -fno-exceptions

PKGCONFIG += \
    libnm \
    glib-2.0

INCLUDEPATH += ../../../../../libnm-kylin/libnm-base-kylin/depend
INCLUDEPATH += ../../../../../libnm-kylin/libnm-base-kylin
INCLUDEPATH += ../../../../../libnm-kylin
INCLUDEPATH += /usr/include/KF5/NetworkManagerQt
INCLUDEPATH += /usr/include/dbus-1.0/dbus


LIBS += -L../../../../../libnm-kylin/libnm-base-kylin -lkylin-nm-base
LIBS += -L/usr/lib/ -lKF5NetworkManagerQt
LIBS += -lgcov



SOURCES += \
    test_kylingeneral.cpp \
    main.cpp

HEADERS += \
    test_kylingeneral.h


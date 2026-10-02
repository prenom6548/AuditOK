TEMPLATE = app
TARGET = test_tcp_link

QT += core gui widgets testlib network dbus
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt

CONFIG += console
CONFIG -= app_bundle

DEFINES += TEST_MODE
QMAKE_CXXFLAGS += --coverage -fbranch-count-reg -fno-exceptions
QMAKE_LDFLAGS += --coverage
LIBS += -lgcov

SOURCES += \
    test_tcp_link.cpp \
    main.cpp \
    simple_tcp_link.cpp

HEADERS += \
    test_tcp_link.h

INCLUDEPATH += \
    ../../../../../.. \
    ../../../../../../kabase \
    ../../../../../../kabase/Qt \
    ../../../../../../src

# 链接所需的库
LIBS += -lpthread

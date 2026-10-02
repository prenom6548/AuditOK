QT       += core gui testlib widgets dbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = test_main
TEMPLATE = app

CONFIG += c++14 qt console warn_on

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_NO_KEYWORDS

SOURCES += test_main.cpp

QMAKE_CXXFLAGS += --coverage
QMAKE_LFLAGS += --coverage -lgcov

LIBS    += -L../../kylin-keyring -lkylin-keyring

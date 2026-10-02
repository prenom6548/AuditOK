QT += core

TARGET = testcurrency
TEMPLATE = app
CONFIG += c++11 console link_pkgconfig

PKGCONFIG += kysdk-ukenv

SOURCES += main.cpp
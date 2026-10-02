QT += widgets

TARGET = testusermanual
TEMPLATE = app

CONFIG += c++11 link_pkgconfig

PKGCONFIG += kysdk-ukenv

SOURCES += main.cpp
QT += widgets

TARGET = testsingleapplication
TEMPLATE = app
CONFIG += c++11 link_pkgconfig

PKGCONFIG += kysdk-alm

SOURCES += main.cpp
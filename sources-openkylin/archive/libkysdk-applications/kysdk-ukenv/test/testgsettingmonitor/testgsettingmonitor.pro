QT += core

TARGET = testgsettingmonitor
TEMPLATE = app
CONFIG += c++11 console link_pkgconfig

PKGCONFIG += gsettings-qt kysdk-ukenv

SOURCES += main.cpp
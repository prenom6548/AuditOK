QT       += core gui widgets testlib dbus

CONFIG += qt console
CONFIG -= app_bundle

CONFIG += c++14 link_pkgconfig

TARGET = test_kylinpassworddialog
TEMPLATE = app

INCLUDEPATH += ../../../../kylin-keyring/kylin-agent
INCLUDEPATH += ../../../../kylin-keyring
INCLUDEPATH += /usr/include/KF5/NetworkManagerQt
INCLUDEPATH += /usr/include/kysdk/applications

LIBS += -L../../../../kylin-keyring/build -lkylin-keyring

PKGCONFIG += gio-2.0 glib-2.0 gio-unix-2.0 libnm libnma libsecret-1 gsettings-qt libcap kysdk-qtwidgets

LIBS += -lgcov

QMAKE_CXXFLAGS += --coverage
QMAKE_LFLAGS += --coverage

SOURCES += test_kylinpassworddialog.cpp

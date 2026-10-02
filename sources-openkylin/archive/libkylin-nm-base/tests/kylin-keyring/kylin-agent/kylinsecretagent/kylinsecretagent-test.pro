QT += core gui dbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = test_kylinsecretagent
TEMPLATE = app

CONFIG += c++14 qt warn_on link_pkgconfig console
CONFIG += no_keywords

PKGCONFIG += gio-2.0 glib-2.0 gio-unix-2.0 libnm libnma libsecret-1 gsettings-qt libcap kysdk-qtwidgets

INCLUDEPATH += /usr/include/KF5/NetworkManagerQt
INCLUDEPATH += ../../../../kylin-keyring
INCLUDEPATH += ../../../../kylin-keyring/kylin-agent
INCLUDEPATH += /usr/src/googletest/googletest/include

LIBS += -L../../../../kylin-keyring/build -lkylin-keyring
LIBS += -L/usr/lib/ -lKF5NetworkManagerQt
LIBS += -lgtest -lpthread
LIBS += --coverage -lgcov

QMAKE_CXXFLAGS += --coverage
QMAKE_LFLAGS += --coverage

SOURCES += main.cpp
SOURCES += test_kylinsecretagent.cpp

HEADERS += test_kylinsecretagent.h

QT += core dbus network
QT -= gui

CONFIG += c++14 qt link_pkgconfig console testcase
CONFIG += --coverage

PKGCONFIG += libnm gsettings-qt gio-2.0 gtk+-3.0

INCLUDEPATH += ../../../../../libnm-kylin/libnm-base-kylin/depend
INCLUDEPATH += ../../../../../libnm-kylin/libnm-base-kylin
INCLUDEPATH += /usr/include/KF5/NetworkManagerQt
INCLUDEPATH += /usr/include/dbus-1.0/dbus
INCLUDEPATH += ../../kt-test-utils/cpp-stub
INCLUDEPATH += ../../kt-test-utils/cpp-stub-ext
INCLUDEPATH += /usr/include/dconf
INCLUDEPATH += /usr/include/libnm
INCLUDEPATH += /usr/include/libmount
INCLUDEPATH += /usr/include/blkid


LIBS += -L../../../../../libnm-kylin/libnm-base-kylin -lkylin-nm-base
LIBS += -L/usr/lib/ -lKF5NetworkManagerQt
LIBS += -lgtest_main -lgtest -lpthread -lgcov

SOURCES += test_kylinwirednetresource.cpp
SOURCES += main.cpp

HEADERS += test_kylinwirednetresource.h
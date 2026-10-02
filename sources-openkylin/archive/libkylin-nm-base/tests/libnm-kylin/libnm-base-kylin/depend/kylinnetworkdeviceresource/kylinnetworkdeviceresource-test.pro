QT       += core dbus network testlib

CONFIG += c++14 qt link_pkgconfig console
CONFIG -= app_bundle

CONFIG += coverage

TARGET = test_kylinnetworkdeviceresource
TEMPLATE = app

PKGCONFIG += gsettings-qt \
	     gio-2.0 \
             libnm \
             gtk+-3.0 \

INCLUDEPATH += ../../../../../libnm-kylin/libnm-base-kylin
INCLUDEPATH += ../../../../../libnm-kylin/libnm-base-kylin/depend
INCLUDEPATH += /usr/include/KF5/NetworkManagerQt
INCLUDEPATH += /usr/include/dbus-1.0/dbus


LIBS    += -L../../../../../libnm-kylin/libnm-base-kylin -lkylin-nm-base
LIBS    += -L/usr/lib/ -lKF5NetworkManagerQt
LIBS    += -lgtest -lgcov -ldconf
LIBS    += -lgio-2.0 -lgobject-2.0 -lglib-2.0


SOURCES +=     test_kylinnetworkdeviceresource.cpp    main.cpp   

HEADERS +=     test_kylinnetworkdeviceresource.h

DEFINES += QT_DEPRECATED_WARNINGS

QT       += core testlib dbus network

QT       -= gui

CONFIG += c++14 qt link_pkgconfig console warn_on no_keywords
CONFIG -= app_bundle

TARGET = test_kyenterpricesettinginfo
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += ../../../../../libnm-kylin/libnm-base-kylin/depend
INCLUDEPATH += ../../../../../libnm-kylin/libnm-base-kylin
INCLUDEPATH += ../../../../../libnm-kylin
INCLUDEPATH += /usr/include/KF5/NetworkManagerQt
INCLUDEPATH += /usr/include/dbus-1.0/dbus



LIBS += -L/usr/lib/ -lKF5NetworkManagerQt
LIBS += -lgcov



#代码覆盖率工具gcov
QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage
QMAKE_CXXFLAGS += --coverage -fno-inline -fno-access-control -fno-exceptions

PKGCONFIG += gsettings-qt \
	         gio-2.0 \
             libnm \
             gtk+-3.0 \
             glib-2.0

#SOURCES += test_kyenterpricesettinginfo.cpp main.cpp

#HEADERS += test_kyenterpricesettinginfo.h

SOURCES += ../../../../../libnm-kylin/libnm-base-kylin/kylinnetworkmanger.cpp \
            ../../../../../libnm-kylin/libnm-base-kylin/depend/kyapnetresource.cpp \
            ../../../../../libnm-kylin/libnm-base-kylin/depend/kyenterpricesettinginfo.cpp \
            ../../../../../libnm-kylin/libnm-base-kylin/depend/kylinactiveconnectresource.cpp \
            ../../../../../libnm-kylin/libnm-base-kylin/depend/kylinconnectoperation.cpp \
            ../../../../../libnm-kylin/libnm-base-kylin/depend/kylinconnectsetting.cpp \
            ../../../../../libnm-kylin/libnm-base-kylin/depend/kylingeneral.cpp \
            ../../../../../libnm-kylin/libnm-base-kylin/depend/kylinnetresource.cpp \
            ../../../../../libnm-kylin/libnm-base-kylin/depend/kylinnetworkdeviceresource.cpp \
            ../../../../../libnm-kylin/libnm-base-kylin/depend/kylinnetworkmanagerutil.cpp \
            ../../../../../libnm-kylin/libnm-base-kylin/depend/kynetloadratethread.cpp \
            ../../../../../libnm-kylin/libnm-base-kylin/depend/kylinnetworkresourcemanager.cpp \
            ../../../../../libnm-kylin/libnm-base-kylin/depend/kylinwiredconnectoperation.cpp \
            ../../../../../libnm-kylin/libnm-base-kylin/depend/kylinwirednetresource.cpp \
            ../../../../../libnm-kylin/libnm-base-kylin/depend/kywirelessconnectoperation.cpp \
            ../../../../../libnm-kylin/libnm-base-kylin/depend/kywirelessnetresource.cpp \
           test_kyenterpricesettinginfo.cpp \
           test_kywirelessconnectoperation.cpp \
           test_kylinnetworkresourcemanager.cpp \
           test_kynetloadratethread.cpp \
           test_kylinconnectoperation.cpp \
           test_kywirelessnetresource.cpp \
           test_kylinnetworkmanagerutil.cpp \
           test_kylinconnectsetting.cpp \
           main.cpp

HEADERS += ../../../../../libnm-kylin/libnm-base-kylin/kylinnetworkmanager.h \
            ../../../../../libnm-kylin/libnm-base-kylin/depend/kynetloadratethread.h \
            ../../../../../libnm-kylin/libnm-base-kylin/libnm-base-kylin_global.h \
            ../../../../../libnm-kylin/libnm-base-kylin/kylin-nm-interface.h \
            ../../../../../libnm-kylin/libnm-base-kylin/depend/kyapnetresource.h \
            ../../../../../libnm-kylin/libnm-base-kylin/depend/kyenterpricesettinginfo.h \
            ../../../../../libnm-kylin/libnm-base-kylin/depend/kylinactiveconnectresource.h \
            ../../../../../libnm-kylin/libnm-base-kylin/depend/kylinapconnectitem.h \
            ../../../../../libnm-kylin/libnm-base-kylin/depend/kylinconnectitem.h \
            ../../../../../libnm-kylin/libnm-base-kylin/depend/kylinconnectoperation.h \
            ../../../../../libnm-kylin/libnm-base-kylin/depend/kylinconnectsetting.h \
            ../../../../../libnm-kylin/libnm-base-kylin/depend/kylingeneral.h \
            ../../../../../libnm-kylin/libnm-base-kylin/depend/kylinnetresource.h \
            ../../../../../libnm-kylin/libnm-base-kylin/depend/kylinnetworkdeviceresource.h \
            ../../../../../libnm-kylin/libnm-base-kylin/depend/kylinnetworkmanagerutil.h \
            ../../../../../libnm-kylin/libnm-base-kylin/depend/kylinnetworkresourcemanager.h \
            ../../../../../libnm-kylin/libnm-base-kylin/depend/kylinnetworkresourcetype.h \
            ../../../../../libnm-kylin/libnm-base-kylin/depend/kylinwiredconnectoperation.h \
            ../../../../../libnm-kylin/libnm-base-kylin/depend/kylinwirednetresource.h \
            ../../../../../libnm-kylin/libnm-base-kylin/depend/kywirelessconnectoperation.h \
            ../../../../../libnm-kylin/libnm-base-kylin/depend/kywirelessnetitem.h \
            ../../../../../libnm-kylin/libnm-base-kylin/depend/kywirelessnetresource.h \
            test_kyenterpricesettinginfo.h \
            test_kywirelessconnectoperation.h \
            test_kylinnetworkresourcemanager.h \
            test_kylinconnectoperation.h \
            test_kywirelessnetresource.h \
            test_kynetloadratethread.h \
            test_kylinnetworkmanagerutil.h \
            test_kylinconnectsetting.h

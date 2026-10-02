#-------------------------------------------------
#
# Project created by QtCreator 2019-08-22T11:12:59
#
#-------------------------------------------------
QT += core dbus xml network
QT -= gui
#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets concurrent sql xml
TEMPLATE = app

TARGET = unit_test_wired
target.source += $$TARGET
target.path = ./

#代码覆盖率工具gcov
QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage
QMAKE_CXXFLAGS += --coverage -fno-inline -fno-access-control -fno-exceptions



LIBS          += -lgtest_main -lpthread
LIBS          +=  -L$$[QT_INSTALL_LIBS] -lgsettings-qt  -lgtest -lgcov -ldconf
LIBS    +=  -L/usr/lib/ -lKF5NetworkManagerQt
#OBJECTS_DIR = ./obj
#MOC_DIR = ./moc
CONFIG += c++14 qt warn_on  no_keywords
CONFIG += create_pc create_prl no_install_prl
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt \
	     gio-2.0 \
             libnm \
             gtk+-3.0 \

INCLUDEPATH += /usr/include/dconf
INCLUDEPATH += /usr/include/KF5/NetworkManagerQt
INCLUDEPATH += /usr/include/dbus-1.0/dbus
#打桩工具
TEST_UTILS_PATH = $$PWD/../kt-test-utils

# Include directories
INCLUDEPATH += $$PROJECT_ROOTDIR \
               $$TEST_UTILS_PATH/cpp-stub \
               $$TEST_UTILS_PATH/cpp-stub-ext \

SOURCES += ../../libnm-kylin/libnm-base-kylin/kylinnetworkmanger.cpp \
            ../../libnm-kylin/libnm-base-kylin/depend/kyapnetresource.cpp \
            ../../libnm-kylin/libnm-base-kylin/depend/kyenterpricesettinginfo.cpp \
            ../../libnm-kylin/libnm-base-kylin/depend/kylinactiveconnectresource.cpp \
            ../../libnm-kylin/libnm-base-kylin/depend/kylinconnectoperation.cpp \
            ../../libnm-kylin/libnm-base-kylin/depend/kylinconnectsetting.cpp \
            ../../libnm-kylin/libnm-base-kylin/depend/kylingeneral.cpp \
            ../../libnm-kylin/libnm-base-kylin/depend/kylinnetresource.cpp \
            ../../libnm-kylin/libnm-base-kylin/depend/kylinnetworkdeviceresource.cpp \
            ../../libnm-kylin/libnm-base-kylin/depend/kylinnetworkmanagerutil.cpp \
            ../../libnm-kylin/libnm-base-kylin/depend/kynetloadratethread.cpp \
            ../../libnm-kylin/libnm-base-kylin/depend/kylinnetworkresourcemanager.cpp \
            ../../libnm-kylin/libnm-base-kylin/depend/kylinwiredconnectoperation.cpp \
            ../../libnm-kylin/libnm-base-kylin/depend/kylinwirednetresource.cpp \
            ../../libnm-kylin/libnm-base-kylin/depend/kywirelessconnectoperation.cpp \
            ../../libnm-kylin/libnm-base-kylin/depend/kywirelessnetresource.cpp \
           unit_test_wired.cpp \
           main.cpp

HEADERS += ../../libnm-kylin/libnm-base-kylin/kylinnetworkmanager.h \
            ../../libnm-kylin/libnm-base-kylin/depend/kynetloadratethread.h \
            ../../libnm-kylin/libnm-base-kylin/libnm-base-kylin_global.h \
            ../../libnm-kylin/libnm-base-kylin/kylin-nm-interface.h \
            ../../libnm-kylin/libnm-base-kylin/depend/kyapnetresource.h \
            ../../libnm-kylin/libnm-base-kylin/depend/kyenterpricesettinginfo.h \
            ../../libnm-kylin/libnm-base-kylin/depend/kylinactiveconnectresource.h \
            ../../libnm-kylin/libnm-base-kylin/depend/kylinapconnectitem.h \
            ../../libnm-kylin/libnm-base-kylin/depend/kylinconnectitem.h \
            ../../libnm-kylin/libnm-base-kylin/depend/kylinconnectoperation.h \
            ../../libnm-kylin/libnm-base-kylin/depend/kylinconnectsetting.h \
            ../../libnm-kylin/libnm-base-kylin/depend/kylingeneral.h \
            ../../libnm-kylin/libnm-base-kylin/depend/kylinnetresource.h \
            ../../libnm-kylin/libnm-base-kylin/depend/kylinnetworkdeviceresource.h \
            ../../libnm-kylin/libnm-base-kylin/depend/kylinnetworkmanagerutil.h \
            ../../libnm-kylin/libnm-base-kylin/depend/kylinnetworkresourcemanager.h \
            ../../libnm-kylin/libnm-base-kylin/depend/kylinnetworkresourcetype.h \
            ../../libnm-kylin/libnm-base-kylin/depend/kylinwiredconnectoperation.h \
            ../../libnm-kylin/libnm-base-kylin/depend/kylinwirednetresource.h \
            ../../libnm-kylin/libnm-base-kylin/depend/kywirelessconnectoperation.h \
            ../../libnm-kylin/libnm-base-kylin/depend/kywirelessnetitem.h \
            ../../libnm-kylin/libnm-base-kylin/depend/kywirelessnetresource.h


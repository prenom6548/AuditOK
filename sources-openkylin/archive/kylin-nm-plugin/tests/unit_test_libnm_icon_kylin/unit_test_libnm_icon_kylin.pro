#-------------------------------------------------
#
# Project created by QtCreator 2019-08-22T11:12:59
#
#-------------------------------------------------
QT += core gui dbus network svg widgets
QT -= gui
TEMPLATE = app

DEFINES += LIBNMICONKYLIN_LIBRARY

CONFIG += c++14 qt link_pkgconfig no_keywords

PKGCONFIG += gsettings-qt \
             kysdk-qtwidgets \
             kylin-nm-base

TARGET = unit_test_libnm_icon_kylin
target.source += $$TARGET
target.path = ./

#代码覆盖率工具gcov
QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage
QMAKE_CXXFLAGS += --coverage -fno-inline -fno-access-control -fno-exceptions

LIBS          += -lgtest_main -lpthread
LIBS          +=  -L$$[QT_INSTALL_LIBS] -lgtest -lgcov
LIBS          +=  -L$$[QT_INSTALL_LIBS] -lkylin-nm-base
LIBS          += -lKF5NetworkManagerQt

#打桩工具
TEST_UTILS_PATH = $$PWD/../kt-test-utils

# Include directories
INCLUDEPATH += $$PROJECT_ROOTDIR \
               $$TEST_UTILS_PATH/cpp-stub \
               $$TEST_UTILS_PATH/cpp-stub-ext \
               /usr/include/KF5

SOURCES +=  main.cpp \
            ../../libnm-kylin/libnm-icon-kylin/kynetworkicon.cpp \
            ../../libnm-kylin/libnm-icon-kylin/imageutil.cpp \
            unit_test_libnm_icon_kylin.cpp \
            ../../plugins/component/DeviceFrame/deviceframe.cpp \
            unit_test_plugin_deviceframe.cpp \
            ../../plugins/component/Divider/divider.cpp \
            unit_test_plugin_divier.cpp \
            ../../plugins/component/FixLabel/fixlabel.cpp \
            unit_test_plugin_fixlabel.cpp \
            ../../plugins/netconnect/itemframe.cpp \
            unit_test_plugin_itemframe.cpp \
            ../../plugins/component/RadioItemButton/radioitembutton.cpp \
            unit_test_plugin_radioitembutton.cpp
#            ../../plugins/component/DrownLabel/drownlabel.cpp
#            unit_test_plugin_drownlabel.cpp
#            ../../plugins/netconnect/netconnect.cpp \
#            unit_test_plugin_netconnect.cpp
#            ../../plugins/netconnect/lanitem.cpp \
#            unit_test_plugin_lanitem.cpp
#            ../../plugins/component/InfoButton/infobutton.cpp \
#            unit_test_plugin_infobutton.cpp \






HEADERS +=  ../../libnm-kylin/libnm-icon-kylin/kynetworkicon.h \
            ../../plugins/component/DeviceFrame/deviceframe.h \
            ../../plugins/component/Divider/divider.h \
            ../../plugins/component/FixLabel/fixlabel.h \
            ../../plugins/netconnect/itemframe.h \
            ../../plugins/component/RadioItemButton/radioitembutton.h
#            ../../plugins/component/DrownLabel/drownlabel.h

#            ../../plugins/netconnect/netconnect.h
#            ../../plugins/netconnect/lanitem.h
#            ../../plugins/component/FixLabel/infobutton.h \



## Default rules for deployment.
#unix {
#    target.path = /usr/lib

#    manager.path = /usr/include/kylin-nm
#    manager.files = kynetworkicon.h

#    INSTALLS += manager
#}
#!isEmpty(target.path): INSTALLS += target

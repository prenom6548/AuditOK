QT       += core gui xml dbus sql network QApt

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = UnitTest_common

QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage
QMAKE_CXXFLAGS += --coverage -fno-inline -fno-access-control -fno-exceptions
CONFIG += c++11

LIBS          += -L$$[QT_INSTALL_LIBS] -lpolkit-qt5-core-1

# 适配窗口管理器圆角阴影
QT += KWindowSystem dbus x11extras

# 适配窗口管理器圆角阴影
LIBS +=-lpthread
LIBS +=-lX11
LIBS +=-ldl -lapt-pkg -lQApt
LIBS +=-lgtest -lgtest_main -lpthread


# 配置gsettings
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt kysdk-soundeffects

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
#DEFINES += QT_DEPRECATED_WARNINGS


DEFINES += QT_NO_INFO_OUTPUT #屏蔽信息输出
DEFINES += QT_NO_WARNING_OUTPUT #屏蔽警告输出

#DEFINES += QT_NO_DEBUG_OUTPUT #屏蔽qDebug输出

#配置sdk
PKGCONFIG += kysdk-qtwidgets kysdk-alm kysdk-log kysdk-waylandhelper
PKGCONFIG += kysdk-kabase
LIBS +=  -L/usr/lib/kysdk/kysdk-base -lkyconf -L/usr/lib/x86_64-linux-gnu/libkysdk-qtwidgets.so

INCLUDEPATH += /usr/include/kysdk/applications/
INCLUDEPATH += /usr/include/kysdk/kysdk-system/
INCLUDEPATH += /usr/include/kysdk/applications/kabase/kylin_system/
INCLUDEPATH += /usr/include/kysdk/applications/
INCLUDEPATH += ../../src/common/utils/

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../../src/common/utils/CustomBoxUtil.cpp \
    ../../src/common/utils/SystemInfoUtil.cpp 
#    ../../src/common/utils/DebUtil.cpp

HEADERS += \
    ../../src/common/GlobalInfo.h \
    ../../src/common/utils/CustomBoxUtil.h \
    ../../src/common/utils/SystemInfoUtil.h 
#    ../../src/common/utils/DebUtil.h

include(../UnitTest.pri)
include(UnitTest_common.pri)

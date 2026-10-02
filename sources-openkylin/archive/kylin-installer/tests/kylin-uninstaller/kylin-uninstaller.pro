#-------------------------------------------------
#
# Project created by QtCreator 2020-12-04T10:41:29
#
#-------------------------------------------------

QT += core gui sql xml svg
CONFIG += c++11 link_pkgconfig
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

LIBS += -L$$[QT_INSTALL_LIBS] -lpolkit-qt5-core-1
LIBS += -ldl

PKGCONFIG += glib-2.0

TARGET = UnitTest_kylin-uninstaller
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# 适配窗口管理器圆角阴影
QT += KWindowSystem dbus x11extras

DEFINES += QT_NO_INFO_OUTPUT #屏蔽信息输出
DEFINES += QT_NO_WARNING_OUTPUT #屏蔽警告输出

#DEFINES += QT_NO_DEBUG_OUTPUT  #屏蔽qDebug输出

# 适配窗口管理器圆角阴影
LIBS +=-lpthread
LIBS +=-lX11

PKGCONFIG += kysdk-qtwidgets kysdk-alm kysdk-log kysdk-waylandhelper
PKGCONFIG += kysdk-kabase
LIBS +=  -L/usr/lib/kysdk/kysdk-base -lkyconf
INCLUDEPATH += ../../uninstall_qt/
INCLUDEPATH += ../../uninstall_qt/utils/


# 配置gsettings
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt

QMAKE_CXXFLAGS += -g

SOURCES += \
    ../../uninstall_qt/lib_fun.cpp \
    ../../uninstall_qt/tool_thread.cpp \
    ../../uninstall_qt/utils/DebUtil.cpp

HEADERS += \
    ../../uninstall_qt/lib_fun.h \
    ../../uninstall_qt/tool_thread.h \
    ../../uninstall_qt/utils/DebUtil.h

include(../UnitTest.pri)
include(UnitTest_kylin-uninstaller.pri)

QT       += core gui dbus network KWindowSystem sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 link_pkgconfig

# 项目名
TARGET = kylin-background-upgrade
TEMPLATE = app

LIBS +=-lukui-log4qt

include(env.pri)
INCLUDEPATH +=./widgets

SOURCES += \
    ./src/core.cpp \
    ./src/main.cpp \
    ./src/traydbus.cpp \
    ./src/trayicon.cpp \
    ./src/updatehandle.cpp \
    ./src/updatewidget.cpp

HEADERS += \
    ./src/core.h \
    ./src/traydbus.h \
    ./src/trayicon.h \
    ./src/updatehandle.h \
    ./src/updatewidget.h

TRANSLATIONS   += translations/kylin-background-upgrade_zh_CN.ts \
                  translations/kylin-background-upgrade_bo_CN.ts

PKGCONFIG  += gsettings-qt kysdk-waylandhelper

FORMS += \
    ./src/updatewidget.ui

RESOURCES += \
    ./data/Resources.qrc

DISTFILES += \
    ./data/update.png

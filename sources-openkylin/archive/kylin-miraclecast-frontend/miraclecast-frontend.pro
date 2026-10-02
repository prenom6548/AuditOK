QT       += core gui dbus KWindowSystem x11extras

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = miraclecast-frontend

# 适配窗口管理器圆角阴影
LIBS +=-lpthread
LIBS +=-lX11

CONFIG += c++11 link_pkgconfig
PKGCONFIG += gsettings-qt
TRANSLATIONS    +=  zh_CN.ts en_US.ts

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dbus/interactiondbusinterface.cpp \
    dbus/interfacedbusproxy.cpp \
    dialog.cpp \
    src/main.cpp \
    src/widget.cpp \
    widget2.cpp \
    xatom-helper.cpp

HEADERS += \
    dbus/interactiondbusinterface.h \
    dbus/interfacedbusproxy.h \
    dialog.h \
    src/widget.h \
    widget2.h \
    xatom-helper.h

FORMS += \
    dialog.ui \
    src/widget.ui \
    widget2.ui


unix {

    target.path = /usr/bin
    INSTALLS += target
}

RESOURCES += \
    pic.qrc

DISTFILES += \
    en_US.ts

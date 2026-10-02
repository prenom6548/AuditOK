QT       += core gui dbus KWindowSystem x11extras

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# 适配窗口管理器圆角阴影
LIBS +=-lpthread
LIBS +=-lX11

CONFIG += c++11 link_pkgconfig
PKGCONFIG += gsettings-qt

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dbus/interactiondbusinterface.cpp \
    dbus/interfacedbusproxy.cpp \
    src/main.cpp \
    src/widget.cpp \
    xatom-helper.cpp

HEADERS += \
    dbus/interactiondbusinterface.h \
    dbus/interfacedbusproxy.h \
    src/widget.h \
    xatom-helper.h

FORMS += \
    src/widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

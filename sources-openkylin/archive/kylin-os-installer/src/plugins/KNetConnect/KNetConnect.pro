QT -= gui
QT += widgets network dbus core

TARGET = KNetConnect
TEMPLATE = lib
DEFINES += KNETCONNECT_LIBRARY
DESTDIR = $$PWD/../../KPlugins

CONFIG += c++11 \
          link_pkgconfig \

PKGCONFIG += kylin-nm-base \
             kysdk-qtwidgets \
             kysdk-widgetutils \
             gio-2.0 \
             gio-unix-2.0 \
             libnm


#LIBS += -llibnm-icon-kylin
#LIBS +=  -L/usr/lib/ -lgsettings-qt -lX11 -lKF5NetworkManagerQt -lukui-log4qt -lkysec

INCLUDEPATH += /usr/include/KF5/NetworkManagerQt \
#PKGCONFIG +=gio-2.0 glib-2.0 gio-unix-2.0 libnm libsecret-1 gtk+-3.0 gsettings-qt libcap kysdk-qtwidgets kysdk-waylandhelper


# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    knetconnect.cpp \
    netframe.cpp

HEADERS += \
    knetconnect_global.h \
    knetconnect.h \
    netframe.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
    INSTALLS += target
}

unix:!macx: LIBS += -L$$PWD/../../KPlugins/ -lReflex

INCLUDEPATH += $$PWD/../Reflex
DEPENDPATH += $$PWD/../Reflex

unix:!macx: LIBS += -L$$PWD/../../KPlugins/ -lPluginDll

INCLUDEPATH += $$PWD/../PluginDll
DEPENDPATH += $$PWD/../PluginDll

unix:!macx: LIBS += -L$$PWD/../../KPlugins/ -lPluginService

INCLUDEPATH += $$PWD/../PluginService
DEPENDPATH += $$PWD/../PluginService


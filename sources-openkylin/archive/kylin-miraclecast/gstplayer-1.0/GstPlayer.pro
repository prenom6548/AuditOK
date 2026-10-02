#-------------------------------------------------
#
# Project created by QtCreator 2019-08-30T14:43:25
#
#-------------------------------------------------

QT       += core gui widgets multimedia multimediawidgets dbus

TARGET = castplayer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += /usr/lib/gcc/x86_64-linux-gnu/9/include

TRANSLATIONS += zh_CN.ts \
                en_US.ts

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11 \
        link_pkgconfig

PKGCONFIG += gstreamer-1.0 \
        gstreamer-video-1.0

SOURCES += \
        main.cpp \
        mobiletype.cpp \
        playwidget.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

FORMS += \
    mobiletype.ui \
    playwidget.ui

HEADERS += \
    mobiletype.h \
    playwidget.h

RESOURCES += \
    pic.qrc \
    translator.qrc

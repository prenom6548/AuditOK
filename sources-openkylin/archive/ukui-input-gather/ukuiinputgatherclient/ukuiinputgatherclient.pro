QT -= gui
QT += x11extras dbus

TEMPLATE = lib
DEFINES += LIBUKUIINPUTGATHER_LIBRARY

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += ../ukui-input-gather/src
LIBS += -lXi

SOURCES += \
    ukuiinputgatherclient.cpp

HEADERS += \
    ukuiinputgatherclient_global.h \
    ukuiinputgatherclient.h

# Default rules for deployment.
unix {
    eventfile.files = ../ukui-input-gather/src/event.h
    eventfile.path  = /usr/include/ukui/

    apifile.files = *.h
    apifile.path  = /usr/include/ukui/

    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target apifile eventfile

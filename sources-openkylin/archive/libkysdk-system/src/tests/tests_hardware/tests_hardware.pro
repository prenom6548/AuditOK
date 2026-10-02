QT += widgets gui dbus

CONFIG += c++14 link_pkgconfig
PKGCONFIG += \
        gtest \
        gtest_main \
        glib-2.0 \
        dbus-1 \
        dbus-glib-1 \
        libnl-3.0 \
        libnl-genl-3.0 \
        libnl-route-3.0 \
        libnm

QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage
QMAKE_CXXFLAGS += --coverage -fno-inline
QMAKE_CFLAGS += --coverage -fno-inline

INCLUDEPATH += /usr/include/kysdk/kysdk-base \
                /usr/include/dbus-1.0/dbus \
                /usr/include/kysdk/kysdk-system \
                $$LIBNL_INCLUDE_DIRS \
                $$LIBNM_INCLUDE_DIRS

LIBS += \
    -lkylog \
    -lkyconf \
    -lpthread \
    -lsystemd \
    -lcups \
    -lcurl \
    -ludev \
    -lusb-1.0 \
    -lbluetooth \
    -lhd \
    -lgobject-2.0 \
    -lm \
    -lX11 \
    -lXrandr \
    -lcjson \
    -lpci \
    -ldbus-glib-1 \
    -lsensors

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += \
                ../../hardware

SOURCES += \
    kyhardware-test.cpp \
    ../../hardware/libkybios.c \
    ../../hardware/libkyboard.c \
    ../../hardware/libkycpu.c \
    ../../hardware/libkydisplay.c \
    ../../hardware/libkync.c \
    ../../hardware/libkyprinter.c \
    ../../hardware/libkyusb.c \
    ../../hardware/libkybluetooth.c \
    ../../hardware/libkyedid.c \
    ../../hardware/libkyfan.c \
    ../../hardware/libkyhw.c

HEADERS += \
    ../../hardware/libkybios.h \
    ../../hardware/libkyboard.h \
    ../../hardware/libkycpu.h \
    ../../hardware/libkydisplay.h \
    ../../hardware/libkync.h \
    ../../hardware/libkyprinter.h \
    ../../hardware/libkyusb.h \
    ../../hardware/libkybluetooth.h \
    ../../hardware/libkyedid.h \
    ../../hardware/libkyfan.h \
    ../../hardware/libkyhw.h

# Default rules for deployment.

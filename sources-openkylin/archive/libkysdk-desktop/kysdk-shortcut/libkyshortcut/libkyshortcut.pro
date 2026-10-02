QT += core dbus widgets 
QT += KGlobalAccel KConfigGui

TEMPLATE = lib
TARGET = kysdk-shortcut
VERSION = 2.5.1.0

CONFIG += c++11 link_pkgconfig
PKGCONFIG += dconf \
             gio-2.0 \
             gsettings-qt

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
    ../utils/kshortcututil.cpp \
    kglobalshortcut.cpp \
    libkyshortcut.cpp \

HEADERS += \
    ../utils/kshortcututil.h \
    kglobalshortcut.h \
    libkyshortcut.h \
    libkykeymap.h

headers.files = $${HEADERS}
headers.path = /usr/include/kysdk/desktop
target.path = $$[QT_INSTALL_LIBS]
INSTALLS += target headers

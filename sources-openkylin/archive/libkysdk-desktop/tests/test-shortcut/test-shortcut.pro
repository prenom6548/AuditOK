QT += testlib widgets gui dbus
QT += KGlobalAccel KConfigGui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

CONFIG += link_pkgconfig
PKGCONFIG += dconf \
             gio-2.0 \
             gsettings-qt

TEMPLATE = app

QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage -fno-inline -fno-access-control -fno-exceptions
QMAKE_CXXFLAGS += --coverage

SOURCES +=  \
        shortcuttest.cpp \
        ../../kysdk-shortcut/libkyshortcut/libkyshortcut.cpp \
        ../../kysdk-shortcut/libkyshortcut/kglobalshortcut.cpp \
        ../../kysdk-shortcut/utils/kshortcututil.cpp

HEADERS += \
        ../../kysdk-shortcut/libkyshortcut/libkyshortcut.h \
        ../../kysdk-shortcut/libkyshortcut/kglobalshortcut.h \
        ../../kysdk-shortcut/libkyshortcut/libkykeymap.h \
        ../../kysdk-shortcut/utils/kshortcututil.h

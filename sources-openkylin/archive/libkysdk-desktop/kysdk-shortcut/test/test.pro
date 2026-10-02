QT -= core dbus
#QT += KGlobalAccel KConfigGui

CONFIG += c++11 console
CONFIG -= app_bundle

CONFIG += link_pkgconfig
#直接编译
#PKGCONFIG += dconf \
#             gio-2.0 \
#             gsettings-qt
#依赖库
PKGCONFIG += kysdk-shortcut
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
        main.cpp \
#        ../libkyshortcut/kglobalshortcut.cpp \
#        ../libkyshortcut/libkyshortcut.cpp \
#        ../utils/kshortcututil.cpp

HEADERS += \
#    ../libkyshortcut/kglobalshortcut.h \
#    ../libkyshortcut/libkykeymap.h \
#    ../libkyshortcut/libkyshortcut.h \
#    ../utils/kshortcututil.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target



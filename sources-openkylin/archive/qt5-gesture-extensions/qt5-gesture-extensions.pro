QT += widgets
TEMPLATE = lib
DEFINES += QT5GESTUREEXTENSIONS_LIBRARY

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
SOURCES += \
    gesture-extension-manager/gesture-extension-manager-private.cpp \
    gesture-extension-manager/gesture-extension-manager.cpp \
    gesture-extensions/slide-gesture.cpp

HEADERS += \
    gesture-extension-manager/gesture-extension-manager-private.h \
    gesture-extension-manager/gesture-extension-manager.h \
    gesture-extensions/gesture-extension.h \
    gesture-extensions/slide-gesture.h \
    qt5-gesture-extensions.h \
    qt5-gesture-extensions_global.h

manpage.files = man/*

# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_LIBS]
    INSTALLS += target

    header.path = /usr/include/qt5-gesture-extensions
    header.files += gesture-extension-manager/gesture-extension-manager.h
    header.files += *_global.h qt5-gesture-extensions.h
    INSTALLS += header

    manpage.path = /usr/share/man/man1
    INSTALLS += manpage
}
#!isEmpty(target.path): INSTALLS += target

QT       += core dbus gui widgets
#CONFIG += console
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

MOC_DIR = ./moctmp
UI_DIR = ./uitmp
RCC_DIR = ./rcctmp
OBJECTS_DIR = ./objtmp

!contains(QMAKE_HOST.arch, sw_64){
    LIBS += -L$$[QT_INSTALL_LIBS] -lukui-log4qt
}
DEFINES+=WITHKYSEC KYDEBUG
#DEFINES+=WITHKYSEC KYDEBUG
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
    checkpower.cpp \
    desktopnotify.cpp \
    main.cpp \
    mainwindow.cpp \
    rebootinstallnotify.cpp

HEADERS += \
    checkpower.h \
    desktopnotify.h \
    mainwindow.h \
    rebootinstallnotify.h

TRANSLATIONS += \
    translations/zh_CN.ts \
    translations/tr.ts \
    translations/bo.ts \
    translations/en_US.ts \
    translations/bo_CN.ts \
    translations/mn.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

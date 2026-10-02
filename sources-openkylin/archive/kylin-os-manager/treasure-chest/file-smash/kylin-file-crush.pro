#-------------------------------------------------
#
# Project created by QtCreator 2015-01-26T09:16:38
#
#-------------------------------------------------

QT       += core
isEqual(QT_MAJOR_VERSION, 5) {
    QT += widgets gui
}
CONFIG +=  c++11 #plugin


TARGET = kylin-file-crush
#TEMPLATE = lib
#DESTDIR = $$_PRO_FILE_PWD_/../

unix {
    UI_DIR = .ui
    MOC_DIR = .moc
    OBJECTS_DIR = .obj
}

target.path = /tmp/$${TARGET}/bin
INSTALLS += target

HEADERS += \
    filewipe.h \
#    shredmanager.h \
    shreddialog.h \
    toolkits.h \
    alertdialog.h \
    utils.h \
    toolkits.h \
    mytitlebar.h \
    systembutton.h \
    kylineditbutton.h \
    mytristatebutton.h \
    shredqthread.h

SOURCES += \
    filewipe.cpp \
    main.cpp \
#    shredmanager.cpp \
    shreddialog.cpp \
    alertdialog.cpp \
    toolkits.cpp \
    mytitlebar.cpp \
    systembutton.cpp \
    kylineditbutton.cpp \
    mytristatebutton.cpp \
    shredqthread.cpp

OTHER_FILES += \
    shred.json

FORMS += \
    alertdialog.ui

RESOURCES += \
    img.qrc

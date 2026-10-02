#-------------------------------------------------
#
# Project created by QtCreator 2019-02-28T14:09:42
#
#-------------------------------------------------

QT       += widgets
QT       += core gui widgets dbus

TEMPLATE = lib
CONFIG += plugin
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt \
             kysdk-qtwidgets
include(../component/drownlabel.pri)

LIBS += -lukcc

TARGET = $$qtLibraryTarget(projection)
DESTDIR = ../..
target.path = $$[QT_INSTALL_LIBS]/ukui-control-center

TRANSLATIONS += translations/zh_CN.ts \
                translations/en_US.ts

isEmpty(PREFIX) {
    PREFIX = /usr
}

qm_files.path = $${PREFIX}/share/projection/translations
qm_files.files = translations/*

CONFIG(release, debug|release) {
    !system($$PWD/translate_generation.sh): error("Failed to generate translation")
}

INCLUDEPATH += \
               $$PROJECT_COMPONENTSOURCE \
               $$PROJECT_ROOTDIR \

INCLUDEPATH += /usr/lib/gcc/aarch64-linux-gnu/9/include/


#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        projection.cpp \
        changeprojectionname.cpp

HEADERS += \
        projection.h \
        changeprojectionname.h

FORMS += \
        projection.ui \
        changeprojectionname.ui \

INSTALLS += target \
            qm_files \

RESOURCES += \
    pic.qrc

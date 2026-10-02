#-------------------------------------------------
#
# Project created by QtCreator 2019-11-27T09:45:26
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = peony-rotate-menu-plugin
TEMPLATE = lib
DEFINES += PEONYENGRAMPAMENUPLUGIN_LIBRARY

include($$PWD/common.pri)

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += link_pkgconfig plugin
PKGCONFIG += peony

include (../codec/kylinimagecodec/kylinimagecodec.pro)

LIBS += -lfreeimage \

#INCLUDEPATH +=  /usr/include/kysdk/kysdk-security/
#LIBS += -L/usr/lib/kysdk/kysdk-security -lkyfilesec


TRANSLATIONS = translations/peony-rotate-menu_zh_CN.ts \
               translations/peony-rotate-menu_tr.ts \
               translations/peony-rotate-menu_cs.ts \
               translations/peony-rotate-menu_zh_HK.ts \
               translations/peony-rotate-menu_bo_CN.ts \
               translations/peony-rotate-menu_mn.ts \
               translations/peony-rotate-menu_ms.ts \
               translations/peony-rotate-menu_vi.ts \
               translations/peony-rotate-menu_th.ts \

SOURCES += \
    rotate-menu-plugin.cpp \
    rotation.cpp

HEADERS += \
    rotate-menu-plugin.h \
    peony-rotate-menu-plugin_global.h \
    rotation.h



unix {
    target.path = $$[QT_INSTALL_LIBS]/peony-extensions
    INSTALLS += target
}

CONFIG += lrelease embed_translations
QM_FILES_RESOURCE_PREFIX = /translations/

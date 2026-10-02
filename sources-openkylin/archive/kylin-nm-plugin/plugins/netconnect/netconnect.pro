QT       += widgets network dbus gui core
TEMPLATE = lib
CONFIG += plugin

include(../component/infobutton.pri)
include(../component/divider.pri)
include(../component/deviceframe.pri)
include(../component/RadioItemButton.pri)
include(../component/fixlabel.pri)
include(netdetails/netdetails.pri)

TARGET = $$qtLibraryTarget(netconnect)
DESTDIR = ../..

CONFIG(release, debug|release) {
    !system($$PWD/translate_generation.sh): error("Failed to generate translation")
}

INCLUDEPATH   +=  \
                 $$PROJECT_COMPONENTSOURCE \
                 $$PROJECT_ROOTDIR \

LIBS          +=  -L$$[QT_INSTALL_LIBS] -lkylin-nm-base

CONFIG += c++11 \
          link_pkgconfig \
          no_keywords

PKGCONFIG += gsettings-qt \
             kysdk-qtwidgets \
             kylin-nm-base

#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    itemframe.cpp \
    lanitem.cpp \
    netconnect.cpp

HEADERS += \
    itemframe.h \
    lanitem.h \
    netconnect.h


TRANSLATIONS += \
        translations/zh_HK.ts \
        translations/zh_Hant.ts \
        translations/zh_CN.ts \
        translations/vi.ts \
        translations/ug.ts \
        translations/ug_CN.ts \
        translations/tr.ts \
        translations/th.ts \
        translations/ms.ts \
        translations/mn.ts \
        translations/ky.ts \
        translations/kk.ts \
        translations/bo.ts \
        translations/bo_CN.ts \
        translations/ar.ts



target.path = /usr/lib/kylin-nm
qm_files.path = $${PREFIX}/share/kylin-nm-plugin/netconnect/
qm_files.files = translations/*.qm

ts_files.path = $${PREFIX}/share/kylin-nm-plugin/netconnect/
ts_files.files = translations/*.ts


INSTALLS += target \
            qm_files \
            ts_files


RESOURCES += \
    resource.qrc

DISTFILES += \
    translations/bo_CN.ts

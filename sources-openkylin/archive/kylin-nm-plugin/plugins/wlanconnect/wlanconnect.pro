QT       += widgets network dbus gui core
TEMPLATE = lib
CONFIG += plugin

include(../component/infobutton.pri)
include(../component/divider.pri)
include(../component/deviceframe.pri)
include(../component/RadioItemButton.pri)
include(../component/fixlabel.pri)
include(../component/fixpushbutton.pri)
include(../kylin-agent/kylin-agent.pri)
include(hiddenwifi/hiddenwifi.pri)

TARGET = $$qtLibraryTarget(wlanconnect)
DESTDIR = ../..

CONFIG(release, debug|release) {
    !system($$PWD/translate_generation.sh): error("Failed to generate translation")
}



INCLUDEPATH   +=  \
                 $$PROJECT_COMPONENTSOURCE \
                 $$PROJECT_ROOTDIR \

LIBS          +=  -L$$[QT_INSTALL_LIBS]  -lkylin-nm-base -lkylin-keyring

CONFIG += c++11 \
          link_pkgconfig \
          no_keywords

PKGCONFIG += gsettings-qt \
             kysdk-qtwidgets \
             kylin-nm-base \
             libsecret-1

#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    itemframe.cpp \
    wlanconnect.cpp \
    wlanitem.cpp \
    oobecheckstyle.cpp \
    checkedlineedit.cpp 

HEADERS += \
    itemframe.h \
    wlanconnect.h \
    wlanitem.h \
    oobecheckstyle.h \
    checkedlineedit.h 

TRANSLATIONS += \
        translations/wlan_zh_HK.ts \
        translations/wlan_zh_Hant.ts \
        translations/wlan_zh_CN.ts \
        translations/wlan_vi.ts \
        translations/wlan_ug.ts \
        translations/wlan_ug_CN.ts \
        translations/wlan_tr.ts \
        translations/wlan_th.ts \
        translations/wlan_ms.ts \
        translations/wlan_mn.ts \
        translations/wlan_ky.ts \
        translations/wlan_kk.ts \
        translations/wlan_bo.ts \
        translations/wlan_bo_CN.ts \
        translations/wlan_ar.ts


target.path = /usr/lib/kylin-nm
qm_files.path = $${PREFIX}/share/kylin-nm-plugin/wlanconnect/
qm_files.files = translations/*.qm

ts_files.path = $${PREFIX}/share/kylin-nm-plugin/wlanconnect/
ts_files.files = translations/*.ts

INSTALLS += target \
            qm_files \
            ts_files

RESOURCES += \
    resource.qrc

DISTFILES += \
    translations/wlan_bo_CN.ts

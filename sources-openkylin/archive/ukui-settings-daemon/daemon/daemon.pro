#-------------------------------------------------
#
# Project created by QtCreator 2020-03-16T09:30:00
#
#-------------------------------------------------
TEMPLATE = app
TARGET = ukui-settings-daemon

QT += core gui dbus
CONFIG += no_keywords link_prl link_pkgconfig c++11
CONFIG -= app_bundle
DEFINES += MODULE_NAME=\\\"Daemon\\\"

QM_FILES_INSTALL_PATH = /usr/share/$${TARGET}/translations/
DEFINES += QM_FILES_INSTALL_PATH='\\"$${QM_FILES_INSTALL_PATH}\\"'

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

include($$PWD/../common/common.pri)

TRANSLATIONS += \
    translations/ukui-settings-daemon_bo_CN.ts \
    translations/ukui-settings-daemon_zh_CN.ts \
    translations/ukui-settings-daemon_mn.ts \
    translations/ukui-settings-daemon_zh_HK.ts \
    translations/ukui-settings-daemon_de.ts \
    translations/ukui-settings-daemon_es.ts \
    translations/ukui-settings-daemon_fr.ts \
    translations/ukui-settings-daemon_kk_KZ.ts \
    translations/ukui-settings-daemon_ky_KG.ts \
    translations/ukui-settings-daemon_ug_CN.ts \
    translations/ukui-settings-daemon_kk.ts \
    translations/ukui-settings-daemon_ky.ts \
    translations/ukui-settings-daemon_ms.ts \
    translations/ukui-settings-daemon_ar.ts \
    translations/ukui-settings-daemon_th.ts \
    translations/ukui-settings-daemon_ug.ts \
    translations/ukui-settings-daemon_vi.ts \
    translations/ukui-settings-daemon_zh_Hant.ts 
    

PKGCONFIG += \
        glib-2.0\
        gio-2.0\
        gobject-2.0\
        gmodule-2.0 \
        dconf

#msgfmt, .po
PO_FILES = $$files(*.po, true)
msgfmt.name = msgfmt
msgfmt.input = PO_FILES
msgfmt.output = ${QMAKE_FILE_IN_PATH}/${QMAKE_FILE_IN_BASE}.mo
msgfmt.commands = msgfmt -o ${QMAKE_FILE_OUT} ${QMAKE_FILE_IN}
msgfmt.CONFIG = no_link
#lrelease, .ts
qtPrepareTool(QMAKE_LRELEASE, lrelease)
lrelease.name = lrelease
lrelease.input = TRANSLATIONS
lrelease.output = ${QMAKE_FILE_IN_PATH}/${QMAKE_FILE_IN_BASE}.qm
lrelease.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_OUT}
lrelease.CONFIG = no_link

QMAKE_EXTRA_COMPILERS += \
    msgfmt \
    lrelease

PRE_TARGETDEPS += \
    compiler_msgfmt_make_all \
    compiler_lrelease_make_all

for (PO_FILE, PO_FILES) {
    MO_DIR = $$dirname(PO_FILE)
    LANG = $$basename(MO_DIR)
    $${LANG}.files = $$replace(PO_FILE, .po, .mo)
    $${LANG}.path = /usr/share/locale/$${LANG}/LC_MESSAGES/
    $${LANG}.CONFIG = no_check_exist
    INSTALLS += $$LANG
}

qm_files.files = $$replace(TRANSLATIONS, .ts, .qm)
qm_files.path = $$QM_FILES_INSTALL_PATH
qm_files.CONFIG = no_check_exist
INSTALLS += qm_files

SOURCES += \
        $$PWD/main.cpp\
        $$PWD/plugin-info.cpp\
        $$PWD/plugin-manager.cpp\
        $$PWD/manager-interface.cpp \
        $$PWD/system-signal-hangler.cpp

HEADERS += \
        $$PWD/plugin-info.h\
        $$PWD/plugin-manager.h\
        $$PWD/manager-interface.h \
        $$PWD/global.h \
        $$PWD/system-signal-hangler.h

target.path = /usr/bin/
target.files = $$OUT_PWD/$$TARGET
!isEmpty(target.path): INSTALLS += target

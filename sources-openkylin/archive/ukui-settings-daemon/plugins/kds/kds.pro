#-------------------------------------------------
#
# Project created by QtCreator 2020-11-12T15:37:31
#
#-------------------------------------------------

QT       += core gui dbus KWindowSystem

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ukydisplayswitch
TEMPLATE = app
CONFIG += C++11 link_pkgconfig

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS MODULE_NAME=\\\"KDS\\\"
QM_FILES_INSTALL_PATH = /usr/share/$${TARGET}/translations/
DEFINES += QM_FILES_INSTALL_PATH='\\"$${QM_FILES_INSTALL_PATH}\\"'

include (qtsingleapplication/qtsingleapplication.pri)

#LIBS += -lX11 -lgsettings-qt

PKGCONFIG += \
    kysdk-waylandhelper \
    kysdk-ukuiwindowhelper \

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

target.source += $$TARGET
target.path = /usr/bin

INSTALLS += target

SOURCES += \
    button.cpp \
    main.cpp \
    screen_switch.cpp \
    style_helper.cpp \
    widget.cpp

HEADERS += \
    button.h \
    screen_switch.h \
    style_helper.h \
    widget.h

RESOURCES += \
    res/img.qrc

TRANSLATIONS += \
    translations/bo_CN.ts \
    translations/zh_CN.ts \
    translations/mn.ts \
    translations/zh_HK.ts \
    translations/de.ts \
    translations/es.ts \
    translations/fr.ts \
    translations/kk_KZ.ts \
    translations/ky_KG.ts \
    translations/ug_CN.ts \
    translations/ar.ts \
    translations/kk.ts \
    translations/ky.ts \
    translations/ms.ts \
    translations/th.ts \
    translations/ug.ts \
    translations/vi.ts \
    translations/zh_Hant.ts 

#lrelease, .ts
qtPrepareTool(QMAKE_LRELEASE, lrelease)
lrelease.name = lrelease
lrelease.input = TRANSLATIONS
lrelease.output = ${QMAKE_FILE_IN_PATH}/${QMAKE_FILE_IN_BASE}.qm
lrelease.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_OUT}
lrelease.CONFIG = no_link

QMAKE_EXTRA_COMPILERS += \
    lrelease

PRE_TARGETDEPS += \
    compiler_lrelease_make_all

qm_files.files = $$replace(TRANSLATIONS, .ts, .qm)
qm_files.path = $$QM_FILES_INSTALL_PATH
qm_files.CONFIG = no_check_exist
INSTALLS += qm_files


include($$PWD/../../common/common.pri)

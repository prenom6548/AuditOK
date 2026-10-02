QT       += core gui dbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TARGET = ukui-power-manager

CONFIG += c++11 link_pkgconfig

PKGCONFIG += gsettings-qt

LIBS += -lukui-log4qt
# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
TRANSLATIONS+=\
    translations/ukui-power-manager_zh_CN.ts \


QM_FILES_INSTALL_PATH = /usr/share/ukui-power-manager/daemon/translations/

# CONFIG += lrelase not work for qt5.6, add those from lrelease.prf for compatibility
qtPrepareTool(QMAKE_LRELEASE, lrelease)
lrelease.name = lrelease
lrelease.input = TRANSLATIONS
lrelease.output = ${QMAKE_FILE_IN_BASE}.qm
lrelease.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_OUT}
lrelease.CONFIG = no_link
QMAKE_EXTRA_COMPILERS += lrelease
PRE_TARGETDEPS += compiler_lrelease_make_all

for (translation, TRANSLATIONS) {
    translation = $$basename(translation)
    QM_FILES += $$OUT_PWD/$$replace(translation, \\..*$, .qm)
}
qm_files.files = $$QM_FILES
qm_files.path = $$QM_FILES_INSTALL_PATH
qm_files.CONFIG = no_check_exist
INSTALLS += qm_files

DEFINES += QM_FILES_INSTALL_PATH='\\"$${QM_FILES_INSTALL_PATH}\\"'

SOURCES += \
    src/brightness.cpp \
    src/config.cpp \
    src/idle.cpp \
    src/lid.cpp \
    src/lowbattery.cpp \
    src/main.cpp \
    src/powermanagercenter.cpp \
    src/powermanagercontrol.cpp \
    src/powerpolicy.cpp

HEADERS += \
    src/brightness.h \
    src/config.h \
    src/error.h \
    src/common.h \
    src/idle.h \
    src/lid.h \
    src/lowbattery.h \
    src/powermanagercenter.h \
    src/powermanagercontrol.h \
    src/powerpolicy.h

# Default rules for deployment.
qnx: target.path = /tmp/usr/bin
else: unix:!android: target.path = /usr/bin
!isEmpty(target.path): INSTALLS += target

desktop.files += resources/ukui-power-manager.desktop

desktop.path = /etc/xdg/autostart/

INSTALLS += desktop

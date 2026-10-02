QT += core gui network KWindowSystem

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = time-shutdown
CONFIG += c++11 link_pkgconfig

PKGCONFIG += gsettings-qt kysdk-qtwidgets kysdk-ukenv kysdk-waylandhelper kysdk-ukuiwindowhelper

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
LIBS += -lX11 -lgio-2.0 -lgobject-2.0 -lglib-2.0 -lukui-log4qt

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.

isEmpty(PREFIX){
    PREFIX = /usr
}

SOURCES += \
    comboxwidget.cpp \
    confirmareawidget.cpp \
    dropdownmenu.cpp \
    main.cpp \
    qtlocalpeer.cpp \
    qtlockedfile.cpp \
    qtsingleapplication.cpp \
    timeshowwidget.cpp \
    verticalscroll60.cpp \
    widget.cpp

HEADERS += \
    comboxwidget.h \
    confirmareawidget.h \
    dropdownmenu.h \
    qtlocalpeer.h \
    qtlockedfile.h \
    qtsingleapplication.h \
    timeshowwidget.h \
    verticalscroll60.h \
    widget.h

target.path = /usr/bin

TRANSLATIONS += \
            translations/time-shutdown_bo_CN.ts \
            translations/time-shutdown_zh_CN.ts \
            translations/time-shutdown_de.ts \
            translations/time-shutdown_es.ts \
            translations/time-shutdown_fr.ts \
            translations/time-shutdown_kk.ts \
            translations/time-shutdown_ky.ts \
            translations/time-shutdown_mn.ts \
            translations/time-shutdown_ug.ts \
            translations/time-shutdown_zh_Hant.ts

for (translation, TRANSLATIONS) {
    translation = $$basename(translation)
    QM_FILES += $$OUT_PWD/$$replace(translation, \\..*$, .qm)
}

QM_FILES_INSTALL_PATH = /usr/share/ukui-time-shutdown/translations/

# CONFIG += lrelase not work for qt5.6, add those from lrelease.prf for compatibility
qtPrepareTool(QMAKE_LRELEASE, lrelease)
lrelease.name = lrelease
lrelease.input = TRANSLATIONS
lrelease.output = ${QMAKE_FILE_IN_BASE}.qm
lrelease.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_OUT}
lrelease.CONFIG = no_link
QMAKE_EXTRA_COMPILERS += lrelease
PRE_TARGETDEPS += compiler_lrelease_make_all

qm_files.files += $$QM_FILES
qm_files.path = $$QM_FILES_INSTALL_PATH
qm_files.CONFIG = no_check_exist

schemes.files += data/org.ukui.time.shutdown.settings.gschema.xml
schemes.path = /usr/share/glib-2.0/schemas/

guide.files += data/time-shutdown
guide.path = /usr/share/kylin-user-guide/data/guide

INSTALLS += target schemes qm_files guide
# Default rules for deployment.
DEFINES += QM_FILES_INSTALL_PATH='\\"$${QM_FILES_INSTALL_PATH}\\"'

DISTFILES += \
    data/org.ukui.time.shutdown.settings.gschema.xml \
    translations/time-shutdown_bo_CN.ts \
    translations/time-shutdown_de.ts \
    translations/time-shutdown_es.ts \
    translations/time-shutdown_fr.ts \
    translations/time-shutdown_kk.ts \
    translations/time-shutdown_ky.ts \
    translations/time-shutdown_mn.ts \
    translations/time-shutdown_ug.ts \
    translations/time-shutdown_zh_CN.ts \
    translations/time-shutdown_zh_Hant.ts

RESOURCES += \
    res.qrc

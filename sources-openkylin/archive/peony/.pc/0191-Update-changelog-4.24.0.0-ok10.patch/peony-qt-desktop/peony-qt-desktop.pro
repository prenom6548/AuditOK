#-------------------------------------------------
#
# Project created by QtCreator 2019-10-28T14:12:53
#
#-------------------------------------------------

QT       += core gui x11extras dbus concurrent KWindowSystem quick quickwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

include(../common.pri)

TARGET = peony-qt-desktop
TEMPLATE = app
QMAKE_CXXFLAGS += -Werror=return-type -Werror=return-local-addr -Werror=uninitialized -Werror=unused-label
PLUGIN_INSTALL_DIRS = $$[QT_INSTALL_LIBS]/peony-extensions
DEFINES += PLUGIN_INSTALL_DIRS='\\"$${PLUGIN_INSTALL_DIRS}\\"'

DEFINES += QT_DEPRECATED_WARNINGS
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include(../libpeony-qt/libpeony-qt-header.pri)
include(../3rd-parties/SingleApplication/singleapplication.pri)
include(../3rd-parties/qtsingleapplication/qtsingleapplication.pri)
include(qml-desktop/qml-desktop.pri)
include(qml-desktop/pag/pag.pri)
include(controls/controls.pri)

#桌面小插件框架
include(desktop-widgets/desktop-widgets.pri)
include(ai-tools/ai-tools.pri)
CONFIG += qmltypes
QML_IMPORT_NAME = org.ukui.peony.desktop
QML_IMPORT_MAJOR_VERSION = 1

DEFINES += QAPPLICATION_CLASS=QApplication

PKGCONFIG +=gio-2.0 glib-2.0 gio-unix-2.0 gsettings-qt libcanberra dconf ukui-quick-framework ukui-quick-core kysdk-qtwidgets
CONFIG += c++17 link_pkgconfig no_keywords lrelease

contains(DEFINES, UKUI_LOG4QT) {
    LIBS += -lukui-log4qt
} else:contains(DEFINES, KY_SDK_KABASE) {
    PKGCONFIG += kysdk-log
}

contains(DEFINES, KY_SDK_WAYLANDHELPER) {
    PKGCONFIG += kysdk-waylandhelper
    PKGCONFIG += kysdk-ukuiwindowhelper
}

contains(DEFINES, KY_SDK_SYSINFO) {
    PKGCONFIG += kysdk-sysinfo
}

LIBS += -L$$PWD/../libpeony-qt/ -lpeony -lX11

TRANSLATIONS += ../translations/peony-qt-desktop/peony-qt-desktop_zh_CN.ts \
                ../translations/peony-qt-desktop/peony-qt-desktop_ms.ts \
                ../translations/peony-qt-desktop/peony-qt-desktop_th.ts \
                ../translations/peony-qt-desktop/peony-qt-desktop_tr.ts \
                ../translations/peony-qt-desktop/peony-qt-desktop_cs.ts \
                ../translations/peony-qt-desktop/peony-qt-desktop_de.ts \
                ../translations/peony-qt-desktop/peony-qt-desktop_es.ts \
                ../translations/peony-qt-desktop/peony-qt-desktop_fr.ts \
                ../translations/peony-qt-desktop/peony-qt-desktop_kk_KZ.ts \
                ../translations/peony-qt-desktop/peony-qt-desktop_ug_CN.ts \
                ../translations/peony-qt-desktop/peony-qt-desktop_ky_KG.ts \
                ../translations/peony-qt-desktop/peony-qt-desktop_bo_CN.ts \
                ../translations/peony-qt-desktop/peony-qt-desktop_mn.ts \
                ../translations/peony-qt-desktop/peony-qt-desktop_vi.ts \
                ../translations/peony-qt-desktop/peony-qt-desktop_zh_HK.ts

SOURCES += \
    advanced-desktop-item-model.cpp \
    background-manager.cpp \
    desktop-window-manager.cpp \
    main.cpp \
    peony-desktop-application.cpp \
    fm-dbus-service.cpp \
    desktop-menu.cpp \
    desktop-menu-plugin-manager.cpp \
    desktop-item-proxy-model.cpp \
    peony-json-operation.cpp \
    bw-list-info.cpp \
    peony-dbus-service.cpp \
    primary-manager.cpp \
    screen-manager.cpp \
    user-dir-manager.cpp \
    textureviewer.cpp \
    texturegrabber.cpp

HEADERS += \
    advanced-desktop-item-model.h \
    background-manager.h \
    common.h \
    desktop-window-manager.h \
    peony-desktop-application.h \
    fm-dbus-service.h \
    desktop-menu.h \
    desktop-menu-plugin-manager.h \
    desktop-item-proxy-model.h \
    peony-json-operation.h \
    bw-list-info.h \
    peony-dbus-service.h \
    primary-manager.h \
    screen-manager.h \
    user-dir-manager.h \
    textureviewer.h \
    texturegrabber.h

target.path = /usr/bin
!isEmpty(target.path): INSTALLS += target

#QM_FILES_RESOURCE_PREFIX = ../translations/peony-qt-desktop
QM_FILES_INSTALL_PATH = /usr/share/peony-qt-desktop

desktop_data.path = /etc/xdg/autostart
desktop_data.files += ../data/peony-desktop.desktop
INSTALLS += desktop_data

DISTFILES +=

RESOURCES += \
    peony-qt-desktop-style.qrc \
    qml.qrc

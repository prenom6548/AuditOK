QT += quick widgets gui

TEMPLATE = lib
CONFIG += plugin

TARGET = desktopimagebackend

include(../common.pri)

RESOURCES += desktopimage.qrc

SOURCES += \
    src/plugin.cpp \
    src/imagebackend.cpp

HEADERS += \
    src/plugin.h \
    src/imagebackend.h

qmldir.files += qmldir
qmldir.path = $$[QT_INSTALL_QML]/org/ukui/desktopimage

target.path = $$[QT_INSTALL_QML]/org/ukui/desktopimage

IMAGE_WIDGET_TS = $$PWD/translations/org.ukui.desktop-image-widget_zh_CN.ts
IMAGE_WIDGET_QM = $$OUT_PWD/translations/org.ukui.desktop-image-widget_zh_CN.qm

release_qm.target = $$IMAGE_WIDGET_QM
release_qm.commands = $$QMAKE_MKDIR $$shell_quote($$OUT_PWD/translations) && $$[QT_INSTALL_BINS]/lrelease $$shell_quote($$IMAGE_WIDGET_TS) -qm $$shell_quote($$IMAGE_WIDGET_QM)
release_qm.depends = $$IMAGE_WIDGET_TS
QMAKE_EXTRA_TARGETS += release_qm
PRE_TARGETDEPS += $$IMAGE_WIDGET_QM

widget_i18n.files += $$IMAGE_WIDGET_QM
widget_i18n.path = /usr/share/ukui/widgets/org.ukui.desktop-image-widget/translations
widget_i18n.CONFIG += no_check_exist

widget.files += \
    metadata.json \
    ui/image-widget-context-menu.qml \
    ui/main.qml \
    ui/preview/small.qml \
    ui/preview/medium.qml \
    ui/preview/large.qml

widget.path = /usr/share/ukui/widgets/org.ukui.desktop-image-widget

INSTALLS += target qmldir widget widget_i18n

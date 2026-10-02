QT       += quick widgets dbus

TEMPLATE = lib
CONFIG  += plugin \
           link_pkgconfig

PKGCONFIG += kysdk-qtwidgets

TARGET = desktopclockbackend

include(../common.pri)

RESOURCES += desktopclock.qrc

SOURCES += \
    src/plugin.cpp \
    src/clockbackend.cpp \
    src/pinyin.cpp

HEADERS += \
    src/plugin.h \
    src/clockbackend.h \
    src/pinyin.h

qmldir.files += qmldir
qmldir.path  = $$[QT_INSTALL_QML]/org/ukui/desktopclock

target.path = $$[QT_INSTALL_QML]/org/ukui/desktopclock

CLOCK_WIDGET_TS = $$PWD/translations/org.ukui.desktop-clock-widget_zh_CN.ts
CLOCK_WIDGET_QM = $$OUT_PWD/translations/org.ukui.desktop-clock-widget_zh_CN.qm

release_qm.target = $$CLOCK_WIDGET_QM
release_qm.commands = $$QMAKE_MKDIR $$shell_quote($$OUT_PWD/translations) && $$[QT_INSTALL_BINS]/lrelease $$shell_quote($$CLOCK_WIDGET_TS) -qm $$shell_quote($$CLOCK_WIDGET_QM)
release_qm.depends = $$CLOCK_WIDGET_TS
QMAKE_EXTRA_TARGETS += release_qm
PRE_TARGETDEPS += $$CLOCK_WIDGET_QM

widget_i18n.files += $$CLOCK_WIDGET_QM
widget_i18n.path = /usr/share/ukui/widgets/org.ukui.desktop-clock-widget/translations
widget_i18n.CONFIG += no_check_exist

widget.files += \
    metadata.json \
    ui/clock-widget-context-menu.qml \
    ui/main.qml \
    ui/AnalogClockFace.qml \
    ui/preview/small.qml \
    ui/preview/medium.qml \
    ui/preview/large.qml \
    fonts/DMSerifDisplay-Regular.ttf

widget.path = /usr/share/ukui/widgets/org.ukui.desktop-clock-widget

INSTALLS += target qmldir widget widget_i18n


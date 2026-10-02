#-------------------------------------------------
#
# Project created by QtCreator 2020-08-12T10:02:10
#
# Jincheng Bai(BaiBai)
# Modify 2020.12.15
#-------------------------------------------------
QT       += core gui dbus sql
QT       += KWindowSystem
TRANSLATIONS    += ./translations/kylin-recorder_zh_CN.ts
QT += multimedia multimediawidgets
QMAKE_CXXFLAGS_RELEASE += -O2
QMAKE_CFLAGS_RELEASE += -O2
QMAKE_LFLAGS += -Wl,--gc-sections -Wl,--as-needed
TARGET = kylin-recorder
TEMPLATE = app
#

INCLUDEPATH += /usr/include/kysdk/applications/ukuiwindowhelper/
LIBS += -lkysdk-ukuiwindowhelper

CONFIG += c++11
CONFIG += link_pkgconfig
PKGCONFIG += peony
PKGCONFIG += kysdk-qtwidgets
PKGCONFIG += kysdk-waylandhelper

INCLUDEPATH += /usr/include/kysdk/kysdk-base/

# 生成ts和qm文件
message($$system(./translations/generate_translations_qm.sh))
message($$system(./translations/generate_translations_ts.sh))

#ffmpeg需要的东西都是
LIBS += -lmp3lame -lavformat -lavcodec

LIBS +=-lX11 \
       -ldl  \
       -lkylog

contains(CONFIG,KAIMING) {
    qm_files.files = translations/*.qm
    qm_files.path = /opt/apps/top.openkylin.kylin-recorder/files
} else {
    DEFINES += NOTKAIMING
    qm_files.files = translations/*.qm
    qm_files.path = /usr/share/kylin-recorder/translations/
}

target.path = /usr/bin
target.source +=$$TARGET

contains(CONFIG,KAIMING) {
    desktop.files = kylin-recorder.desktop
    desktop.path = /opt/apps/top.openkylin.kylin-recorder/files/share/applications
    appdesktop.files += kylin-recorder-kaiming.desktop
    appdesktop.path = /opt/apps/top.openkylin.kylin-recorder/files/share/applications
    icons.files = img/kylin-recorder.svg
    icons.path = /opt/apps/top.openkylin.kylin-recorder/files/share/pixmaps
    schemes.files += \
        data/org.kylin-recorder-data.gschema.xml \
        data/org.ukui.log4qt.kylin-recorder.gschema.xml
    schemes.path = /opt/apps/top.openkylin.kylin-recorder/files/share/glib-2.0/schemas/
    guide.files = data/kylin-recorder/
    guide.path = /opt/apps/top.openkylin.kylin-recorder/files/share/kylin-user-guide/data/guide/
} else{
    desktop.files = kylin-recorder.desktop
    desktop.path = /usr/share/applications/
    appdesktop.files += kylin-recorder.desktop
    appdesktop.path = /usr/share/applications/
    icons.files = svg/recording_128.svg
    icons.path = /usr/share/pixmaps/
    schemes.files += \
        $$PWD/data/org.kylin-recorder-data.gschema.xml \
        $$PWD/data/org.ukui.log4qt.kylin-recorder.gschema.xml
    schemes.path = /usr/share/glib-2.0/schemas/
    guide.files = data/kylin-recorder/
    guide.path = /usr/share/kylin-user-guide/data/guide/
}

# gsettings

#schemes.files = data/org.kylin-recorder-data.gschema.xml

INSTALLS += target desktop appdesktop icons schemes qm_files guide

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.

CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt kysdk-qtwidgets kysdk-waylandhelper kysdk-datacollect
LIBS += -L/usr/lib/kysdk/kysdk-base  -lkylog -lkyconf

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        src/audiofileprocess.cpp \
        src/clipbutton.cpp \
        src/clipwidget.cpp \
        src/custombutton.cpp \
        src/customslider.cpp \
        src/daemondbus.cpp \
        src/dbusadapter.cpp \
        src/ffutil.cpp \
        src/fileitem.cpp \
        src/lineedit.cpp \
        src/log.cpp \
        src/main.cpp \
        src/mainwindow.cpp \
        src/menumodule.cpp \
        src/miniwindow.cpp \
        src/mmediaplayer.cpp \
        src/mmediaplaylist.cpp \
        src/mp3encoder.cpp \
        src/mylabel.cpp \
        src/mythread.cpp \
        src/mywave.cpp \
        src/playcontrol.cpp \
        src/recorderdatabase.cpp \
        src/renamewidget.cpp \
        src/save.cpp \
        src/scopeframe.cpp \
        src/settings.cpp \
        src/tipwindow.cpp \
        src/tools.cpp \
        src/xatom-helper.cpp \
        src/mpvlib/mpvlibload.cpp

HEADERS += \
        src/audiofileprocess.h \
        src/clipbutton.h \
        src/clipwidget.h \
        src/custombutton.h \
        src/customslider.h \
        src/daemondbus.h \
    src/dbusadapter.h \
        src/ffutil.h \
        src/fileitem.h \
        src/lineedit.h \
    src/log.h \
        src/mainwindow.h \
        src/menumodule.h \
        src/miniwindow.h \
        src/mmediaplayer.h \
        src/mmediaplaylist.h \
        src/mp3encoder.h \
        src/mylabel.h \
        src/mythread.h \
        src/mywave.h \
        src/playcontrol.h \
        src/recorderdatabase.h \
        src/renamewidget.h \
        src/save.h \
        src/scopeframe.h \
        src/settings.h \
        src/tipwindow.h \
        src/tools.h \
        src/mpvlib/mpvlibload.h \
        src/mpvlib/qthelper.h

RESOURCES += \
    picture.qrc

DISTFILES += \
    data/org.kylin-recorder-data.gschema.xml \
    data/org.ukui.log4qt.kylin-recorder.gschema.xml \
    translations/kylin-recorder_zh_CN.qm \
    translations/kylin-recorder_zh_CN.ts \
    translations/qt_zh_CN.qm

DEFINES += KY_SDK_DATE
PKGCONFIG += kysdk-systime

CONFIG += c++11 link_pkgconfig
PKGCONFIG += kysdk-datacollect

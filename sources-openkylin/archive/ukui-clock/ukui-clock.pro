#-------------------------------------------------
#
# Project created by QtCreator 2014-06-28T21:00:26
#
#-------------------------------------------------

QT       += core gui multimedia sql

# 适配窗口管理器圆角阴影
QT += KWindowSystem dbus
QMAKE_CXXFLAGS_RELEASE += -O2
QMAKE_CFLAGS_RELEASE += -O2
QMAKE_LFLAGS += -Wl,--gc-sections -Wl,--as-needed
# 适配窗口管理器圆角阴影
LIBS +=-lpthread
# LIBS +=-lX11
# LIBS += -lukui-log4qt
# 配置gsettings
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt
PKGCONFIG += kysdk-waylandhelper

contains(CONFIG, KAIMING) {
    music.path = /opt/apps/top.openkylin.ukui-clock/files/share/ukui-clock/
    translation.path = /opt/apps/top.openkylin.ukui-clock/files
    guide.path =  /opt/apps/top.openkylin.ukui-clock/files/share/kylin-user-guide/data/guide/kylin-alarm-clock/
    desktopfile.path = /opt/apps/top.openkylin.ukui-clock/files/share/applications/
    target.path = /opt/apps/top.openkylin.ukui-clock/files/bin
    dbusService.path = /opt/apps/top.openkylin.ukui-clock/files/share/dbus-1/services/
} else {
    DEFINES += NOTKAIMING
    PKGCONFIG += kysdk-ukuiwindowhelper
    dbusService.path = /usr/share/dbus-1/services/
    music.path = /usr/share/ukui-clock/
    translation.path = /usr/share/ukui-clock/ukui31
    guide.path =  /usr/share/kylin-user-guide/data/guide/kylin-alarm-clock/
    desktopfile.path = /usr/share/applications/
    target.path = /usr/bin/

}

dbusService.files = debian/ukui-clock/usr/share/dbus-1/services/org.kylin.dbus.ukuiclock.service
INSTALLS += dbusService

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ukui-clock
TEMPLATE = app


CONFIG += c++11 link_pkgconfig
PKGCONFIG += gsettings-qt kysdk-qtwidgets

INCLUDEPATH += /usr/include/kysdk/kysdk-base/
# 解析音频文件
LIBS += -lavformat \
        -lkylog

SOURCES += \
    adaptscreeninfo.cpp \
    baseverticalscroll.cpp \
    clock.cpp \
    clockentitydao.cpp \
    clockdbusadaptor.cpp \
    commontooltip.cpp \
    coreplayer/mmediaplayer.cpp \
    coreplayer/mmediaplaylist.cpp \
    coreplayer/mpvlibload.cpp \
    coreplayer/playcontroller.cpp \
    countdownAnimation.cpp \
    customcombox.cpp \
    customlistwidget.cpp \
    dbusbaseclockadaptor.cpp \
    dbusdeleteclockbyidadaptor.cpp \
    dbusnoticeshowagainadaptor.cpp \
    dbusselectclockbyidadaptor.cpp \
    dbusupdateclockadaptor.cpp \
    deleteMsg.cpp \
    fieldvalidutil.cpp \
    globalinstance.cpp \
    gsettingsubject.cpp \
    itemNew.cpp \
    kylinqfiledialog.cpp \
    main.cpp \
    noticeAlarm.cpp \
    noticealarmpoolutil.cpp \
    primarymanager.cpp \
    qroundProgressBar.cpp \
    selectbtn.cpp \
    selectbtnutil.cpp \
    setAlarmRepeatDialog.cpp \
    singleApplication.cpp \
    stopwatchItem.cpp \
    theme.cpp \
    utils.cpp \
    verticalScroll24.cpp \
    verticalScroll60.cpp \
    verticalScroll99.cpp \
    tinycountdown.cpp \
    CJsonObject.cpp \
    cJSON.c \
    verticalscrollapm.cpp \
    kabase/log.cpp \

TRANSLATIONS += translations/ukui-clock_tr.ts \
                translations/ukui-clock_bo_CN.ts \
                translations/ukui-clock_de.ts \
                translations/ukui-clock_es.ts \
                translations/ukui-clock_fr.ts \
                translations/ukui-clock_ky.ts \
                translations/ukui-clock_kk.ts \
                translations/ukui-clock_mn.ts \
                translations/ukui-clock_ug_CN.ts \
                translations/ukui-clock_zh_CN.ts \
                translations/ukui-clock_zh_HK.ts \
                translations/ukui-clock_vi.ts \
                translations/ukui-clock_ms.ts \
                translations/ukui-clock_th.ts


HEADERS  += clock.h \
    ClockInterface.h \
    adaptscreeninfo.h \
    baseverticalscroll.h \
    clockentitydao.h \
    clockdbusadaptor.h \
    commontooltip.h \
    connection.h \
    constant_class.h \
    coreplayer/mmediaplayer.h \
    coreplayer/mmediaplaylist.h \
    coreplayer/mpvlibload.h \
    coreplayer/playcontroller.h \
    coreplayer/qthelper.h \
    countdownAnimation.h \
    customcombox.h \
    customlistwidget.h \
    dbusbaseclockadaptor.h \
    dbusdeleteclockbyidadaptor.h \
    dbusnoticeshowagainadaptor.h \
    dbusselectclockbyidadaptor.h \
    dbusupdateclockadaptor.h \
    deleteMsg.h \
    fieldvalidutil.h \
    globalinstance.h \
    gsettingsubject.h \
    itemNew.h \
    kylinqfiledialog.h \
    noticeAlarm.h \
    noticealarmpoolutil.h \
    primarymanager.h \
    qroundProgressBar.h \
    selectbtn.h \
    selectbtnutil.h \
    setAlarmRepeatDialog.h \
    singleApplication.h \
    stopwatchItem.h \
    theme.h \
    utils.h \
    verticalScroll24.h \
    verticalScroll60.h \
    verticalScroll99.h \
    tinycountdown.h \
    CJsonObject.hpp \
    cJSON.h \
    verticalscrollapm.h \
    kabase/log.h



FORMS    += clock.ui \
    deleteMsg.ui \
    noticeAlarm.ui \
    tinycountdown.ui


RESOURCES += \
    images.qrc

RC_FILE = clock.rc

unix {
    INSTALLS += target

    music.files += music/bark.wav
    music.files += music/drip.wav
    music.files += music/glass.wav
    music.files += music/sonar.wav
    INSTALLS += music

    translation.files += translations/*.qm
    INSTALLS += translation

    
    guide.files += guide/*
    INSTALLS += guide


}


desktopfile.files = ukui-clock.desktop
INSTALLS += desktopfile



DISTFILES += \
    clock_conf.ini \
    image/DFPKingGothicGB-Semibold-2.ttf \
    image/noClockBlack.svg \
    music/bark.wav \
    music/drip.wav \
    music/glass.wav \
    music/sonar.wav

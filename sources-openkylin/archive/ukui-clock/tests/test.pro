#-------------------------------------------------
#
# Project created for all unit tests
#
#-------------------------------------------------

QT       += core gui multimedia sql testlib

# 适配窗口管理器圆角阴影
QT += KWindowSystem dbus
# 适配窗口管理器圆角阴影
LIBS +=-lpthread \
       -lukui-log4qt

# 配置gsettings
CONFIG += link_pkgconfig c++11 
PKGCONFIG += gsettings-qt kysdk-waylandhelper kysdk-ukuiwindowhelper kysdk-qtwidgets
QMAKE_CXXFLAGS += --coverage -fno-exceptions -fbranch-count-reg
QMAKE_LFLAGS += --coverage -lgcov -fprofile-arcs -ftest-coverage


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += testcase

TARGET = test

INCLUDEPATH += /usr/include/kysdk/kysdk-base/ \
              ../  \
              ../coreplayer 

# 解析音频文件
LIBS += -lavformat \
        -lkylog

FORMS    += ../clock.ui \
    ../deleteMsg.ui \
    ../noticeAlarm.ui \
    ../tinycountdown.ui

# 核心修复：所有列表项末尾（除最后一行）都加反斜杠，格式统一
SOURCES += \
    $$PWD/mainTest.cpp \
    $$PWD/testclass.cpp \
     ../adaptscreeninfo.cpp \
    ../baseverticalscroll.cpp \
    ../clock.cpp \
    ../clockentitydao.cpp \
    ../clockdbusadaptor.cpp \
    ../commontooltip.cpp \
    ../coreplayer/mmediaplayer.cpp \
    ../coreplayer/mmediaplaylist.cpp \
    ../coreplayer/mpvlibload.cpp \
    ../coreplayer/playcontroller.cpp \
    ../countdownAnimation.cpp \
    ../customcombox.cpp \
    ../customlistwidget.cpp \
    ../dbusbaseclockadaptor.cpp \
    ../dbusdeleteclockbyidadaptor.cpp \
    ../dbusnoticeshowagainadaptor.cpp \
    ../dbusselectclockbyidadaptor.cpp \
    ../dbusupdateclockadaptor.cpp \
    ../deleteMsg.cpp \
    ../fieldvalidutil.cpp \
    ../globalinstance.cpp \
    ../gsettingsubject.cpp \
    ../integer.cpp \
    ../itemNew.cpp \
    ../kylinqfiledialog.cpp \
    ../noticeAlarm.cpp \
    ../noticealarmpoolutil.cpp \
    ../primarymanager.cpp \
    ../qroundProgressBar.cpp \
    ../selectbtn.cpp \
    ../selectbtnutil.cpp \
    ../setAlarmRepeatDialog.cpp \
    ../singleApplication.cpp \
    ../stopwatchItem.cpp \
    ../theme.cpp \
    ../utils.cpp \
    ../verticalScroll24.cpp \
    ../verticalScroll60.cpp \
    ../verticalScroll99.cpp \
    ../tinycountdown.cpp \
    ../CJsonObject.cpp \
    ../cJSON.c \
    ../verticalscrollapm.cpp \
    ../kabase/log.cpp  

HEADERS  += $$PWD/testclass.h \
    ../clock.h \
    ../ClockInterface.h \
    ../adaptscreeninfo.h \
    ../baseverticalscroll.h \
    ../clockentitydao.h \
    ../clockdbusadaptor.h \
    ../commontooltip.h \
    ../connection.h \
    ../constant_class.h \
    ../coreplayer/mmediaplayer.h \
    ../coreplayer/mmediaplaylist.h \
    ../coreplayer/mpvlibload.h \
    ../coreplayer/playcontroller.h \
    ../coreplayer/qthelper.h \
    ../countdownAnimation.h \
    ../customcombox.h \
    ../customlistwidget.h \
    ../dbusbaseclockadaptor.h \
    ../dbusdeleteclockbyidadaptor.h \
    ../dbusnoticeshowagainadaptor.h \
    ../dbusselectclockbyidadaptor.h \
    ../dbusupdateclockadaptor.h \
    ../deleteMsg.h \
    ../fieldvalidutil.h \
    ../globalinstance.h \
    ../gsettingsubject.h \
    ../integer.h \
    ../itemNew.h \
    ../kylinqfiledialog.h \
    ../noticeAlarm.h \
    ../noticealarmpoolutil.h \
    ../primarymanager.h \
    ../qroundProgressBar.h \
    ../selectbtn.h \
    ../selectbtnutil.h \
    ../setAlarmRepeatDialog.h \
    ../singleApplication.h \
    ../stopwatchItem.h \
    ../theme.h \
    ../utils.h \
    ../verticalScroll24.h \
    ../verticalScroll60.h \
    ../verticalScroll99.h \
    ../tinycountdown.h \
    ../CJsonObject.hpp \
    ../cJSON.h \
    ../verticalscrollapm.h \
    ../kabase/log.h






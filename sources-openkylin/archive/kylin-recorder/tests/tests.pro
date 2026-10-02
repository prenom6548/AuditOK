#-------------------------------------------------
#
# Project created by QtCreator 2020-08-12T10:02:10
#
# Jincheng Bai(BaiBai)
# Modify 2020.12.15
#-------------------------------------------------
QT       += core gui dbus sql testlib
QT       += KWindowSystem
QT += multimedia multimediawidgets

DEFINES += WHITE_TEST

QMAKE_LFLAGS += -Wl,--gc-sections -Wl,--as-needed
TARGET = tests
#TEMPLATE = app

QMAKE_CXXFLAGS += --coverage -fno-exceptions
QMAKE_LFLAGS += --coverage -lgcov



INCLUDEPATH += /usr/include/kysdk/applications/ukuiwindowhelper/
LIBS += -lkysdk-ukuiwindowhelper

CONFIG += c++11
CONFIG += link_pkgconfig testcase
PKGCONFIG += peony
PKGCONFIG += kysdk-qtwidgets
PKGCONFIG += kysdk-waylandhelper

INCLUDEPATH += /usr/include/kysdk/kysdk-base/ ../src/ ../src/mpvlib

# 生成ts和qm文件
#message($$system(./translations/generate_translations_qm.sh))
#message($$system(./translations/generate_translations_ts.sh))

#ffmpeg需要的东西都是
LIBS += -lmp3lame -lavformat -lavcodec

LIBS += -ldl  \
       -lkylog


# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.

PKGCONFIG += gsettings-qt kysdk-qtwidgets kysdk-waylandhelper kysdk-datacollect kysdk-systime
LIBS += -L/usr/lib/kysdk/kysdk-base  -lkylog -lkyconf

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        ../src/audiofileprocess.cpp \
        ../src/clipbutton.cpp \
        ../src/clipwidget.cpp \
        ../src/custombutton.cpp \
        ../src/customslider.cpp \
        ../src/daemondbus.cpp \
        ../src/dbusadapter.cpp \
        ../src/ffutil.cpp \
        ../src/fileitem.cpp \
        ../src/lineedit.cpp \
        ../src/log.cpp \
        ../src/mainwindow.cpp \
        ../src/menumodule.cpp \
        ../src/miniwindow.cpp \
        ../src/mmediaplayer.cpp \
        ../src/mmediaplaylist.cpp \
        ../src/mp3encoder.cpp \
        ../src/mylabel.cpp \
        ../src/mythread.cpp \
        ../src/mywave.cpp \
        ../src/playcontrol.cpp \
        ../src/recorderdatabase.cpp \
        ../src/renamewidget.cpp \
        ../src/save.cpp \
        ../src/scopeframe.cpp \
        ../src/settings.cpp \
        ../src/tipwindow.cpp \
        ../src/tools.cpp \
        ../src/xatom-helper.cpp  \
        ../src/mpvlib/mpvlibload.cpp \
        mainTest.cpp    \
        test_mainwindow.cpp \
        test_clipbutton.cpp \
        test_lineedit.cpp \
        test_audiofileprocess.cpp \
        test_mp3encoder.cpp


HEADERS += \        
        ../src/audiofileprocess.h \
        ../src/clipbutton.h \
        ../src/clipwidget.h \
        ../src/custombutton.h \
        ../src/customslider.h \
        ../src/daemondbus.h \
    ../src/dbusadapter.h \
        ../src/ffutil.h \
        ../src/fileitem.h \
        ../src/lineedit.h \
    ../src/log.h \
        ../src/mainwindow.h \
        ../src/menumodule.h \
        ../src/miniwindow.h \
        ../src/mmediaplayer.h \
        ../src/mmediaplaylist.h \
        ../src/mp3encoder.h \
        ../src/mylabel.h \
        ../src/mythread.h \
        ../src/mywave.h \
        ../src/playcontrol.h \
        ../src/recorderdatabase.h \
        ../src/renamewidget.h \
        ../src/save.h \
        ../src/scopeframe.h \
        ../src/settings.h \
        ../src/tipwindow.h \
        ../src/tools.h \
        ../src/mpvlib/mpvlibload.h \
        ../src/mpvlib/qthelper.h \
        test_mainwindow.h \
        test_clipbutton.h \
        test_lineedit.h \
        test_audiofileprocess.h \
        test_mp3encoder.h

DEFINES += KY_SDK_DATE

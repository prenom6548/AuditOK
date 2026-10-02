QT       += core gui sql widgets dbus x11extras KWindowSystem network svg

#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

LIBS += -lpeony \

#LIBS += -L/usr/lib/libukui-log4qt.so.1.0.0 -lukui-log4qt

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS _UNICODE

# about widget version
#VERSION = 1.1.0.1-0k0.7
#DEFINES += APP_VERSION=\\\"$$VERSION\\\"

TRANSLATIONS += \
            translations/kylin-music_zh_CN.ts \
            translations/kylin-music_bo_CN.ts \
            translations/kylin-music_mn.ts \
			translations/kylin-music_zh_HK.ts
!system($$PWD/translations/generate_translations_pm.sh): error("Failed to generate qm")

contains(CONFIG, KAIMING) {
    target.path = /opt/apps/top.openkylin.kylin-music/files/bin
    target.source += $$TARGET

    icon.path = /opt/apps/top.openkylin.kylin-music/files/share/pixmaps
    icon.files = img/kylin-music.png

    desktop.path = /opt/apps/top.openkylin.kylin-music/files/share/applications/
    desktop.files = kylin-music.desktop

    schemes.files += \
        data/org.kylin-music-data.gschema.xml \
        data/org.ukui.log4qt.kylin-music.gschema.xml
    schemes.path = /opt/apps/top.openkylin.kylin-music/files/share/glib-2.0/schemas/

    # simple.files = $$PWD/3rdparty/build/src/libsimple.so
    # simple.path = /opt/apps/top.openkylin.kylin-music/files/lib

#    dict.files +=$$PWD/3rdparty/build/cppjieba/dict
#    dict.path = /opt/apps/top.openkylin.kylin-music/files/share/kylin-music

    qm_files.files = translations/*.qm
    qm_files.path = /opt/apps/top.openkylin.kylin-music/files/share/translations

    help_files.files = data/kylin-music/
    help_files.path = /opt/apps/top.openkylin.kylin-music/files/share/kylin-user-guide/data/guide/
} else {
    target.path = /usr/bin
    target.source += $$TARGET

    icon.path = /usr/share/pixmaps
    icon.files = img/kylin-music.png

    desktop.path = /usr/share/applications/
    desktop.files = kylin-music.desktop

    schemes.files += \
        data/org.kylin-music-data.gschema.xml \
        data/org.ukui.log4qt.kylin-music.gschema.xml
    schemes.path = /usr/share/glib-2.0/schemas/

    simple.files = $$PWD/3rdparty/build/src/libsimple.so
    simple.path = /usr/lib/kylin-music

    #dict.files +=$$PWD/3rdparty/build/cppjieba/dict
    #dict.path = /usr/share/kylin-music

    qm_files.files = translations/*.qm
    qm_files.path = /usr/share/kylin-music/translations/

    help_files.files = data/kylin-music/
    help_files.path = /usr/share/kylin-user-guide/data/guide/


}


INSTALLS += \
    target  \
    icon    \
    desktop \
    schemes \
    simple \
#    dict \
    qm_files\
    help_files

CONFIG += link_pkgconfig

PKGCONFIG += gsettings-qt taglib gio-unix-2.0 kysdk-sysinfo kysdk-qtwidgets  kysdk-waylandhelper kysdk-ukuiwindowhelper kysdk-datacollect

contains(CONFIG, KAIMING)  {
    message("KAIMING")
} else {
    message("NOT KAIMING")
    DEFINES += NOTKAIMING
}

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
INCLUDEPATH += qtsingleapplication
DEPENDPATH += qtsingleapplication
# 适配窗口管理器圆角阴影
LIBS +=-lpthread
LIBS +=-lX11
# 虚拟键盘按键
LIBS +=-lXtst

# 解析音频文件
LIBS += -lavformat -lavcodec -lavutil

#LIBS += -lsimple
LIBS += -lsqlite3

SOURCES += \
    UI/base/customLabel.cpp \
    UI/base/kaction.cpp \
    UI/base/log.cpp \
    UI/base/newSonglistLineEdit.cpp \
    UI/base/popupDialog.cpp \
    UI/base/widgetstyle.cpp \
    UI/base/promptwidget.cpp \
    UI/dbusadapter.cpp \
    UI/globalsignal.cpp \
    UI/mainwidget.cpp \
    UI/player/miniWidget.cpp \
    UI/player/musicslider.cpp \
    UI/player/playbackmodewidget.cpp \
    UI/player/playsongarea.cpp \
    UI/player/searchedit.cpp \
    UI/player/searchresult.cpp \
    UI/player/sliderwidget.cpp \
    UI/player/lyricsdialog.cpp \
    UI/player/lyricswidget.cpp \
    UI/player/lrcparser.cpp \
    UI/base/xatom-helper.cpp \
    UI/search/musicsearchlistdelegate.cpp \
    UI/search/musicsearchlistmodel.cpp \
    UI/search/musicsearchlistview.cpp \
    UI/sidebar/customSiderBarScrollArea.cpp \
    UI/sidebar/customToolButton.cpp \
    UI/sidebar/customtransparentwidget.cpp \
    UI/sidebar/leftsiderbarwidget.cpp \
    UI/sidebar/sidebarwidget.cpp \
    UI/tableview/customTreeView.cpp \
    UI/tableview/customTreeViewDelegate.cpp \
    UI/tableview/tablehistory.cpp \
    UI/tableview/tableone.cpp \
    UI/titlebar/menumodule.cpp \
    UI/titlebar/titlebar.cpp \
    UIControl/base/daemonipcdbus.cpp \
    UIControl/base/musicDataBase.cpp \
    UIControl/base/musicfileinformation.cpp \
    UIControl/player/coreplayer/mediaplaylist.cpp \
    UIControl/player/coreplayer/mpvcore.cpp \
    UIControl/player/coreplayer/mpvlibload.cpp \
    UIControl/player/player.cpp \
    UIControl/tableview/musiclistmodel.cpp \
    main.cpp



HEADERS += \
    UI/base/customLabel.h \
    UI/base/kaction.h \
    UI/base/log.h \
    UI/base/newSonglistLineEdit.h \
    UI/base/popupDialog.h \
    UI/base/widgetstyle.h \
    UI/base/promptwidget.h \
    UI/dbusadapter.h \
    UI/globalsignal.h \
    UI/mainwidget.h \
    UI/player/miniWidget.h \
    UI/player/musicslider.h \
    UI/player/playbackmodewidget.h \
    UI/player/playsongarea.h \
    UI/player/searchedit.h \
    UI/player/searchresult.h \
    UI/player/sliderwidget.h \
    UI/player/lyricsdialog.h \
    UI/player/lyricswidget.h \
    UI/player/lrcparser.h \
    UI/base/xatom-helper.h \
    UI/search/musicsearchlistdelegate.h \
    UI/search/musicsearchlistmodel.h \
    UI/search/musicsearchlistview.h \
    UI/sidebar/customSiderBarScrollArea.h \
    UI/sidebar/customToolButton.h \
    UI/sidebar/customtransparentwidget.h \
    UI/sidebar/leftsiderbarwidget.h \
    UI/sidebar/sidebarwidget.h \
    UI/tableview/customTreeView.h \
    UI/tableview/customTreeViewDelegate.h \
    UI/tableview/tablehistory.h \
    UI/tableview/tableone.h \
    UI/titlebar/menumodule.h \
    UI/titlebar/titlebar.h \
    UIControl/base/daemonipcdbus.h \
    UIControl/base/musicDataBase.h \
    UIControl/base/musicfileinformation.h \
    UIControl/player/coreplayer/mediaplaylist.h \
    UIControl/player/coreplayer/mpvcore.h \
    UIControl/player/coreplayer/mpvlibload.h \
    UIControl/player/coreplayer/qthelper.h \
    UIControl/player/player.h \
    UIControl/tableview/musiclistmodel.h \


# Default rules for deployment.
#qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /opt/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

DISTFILES += \
    data/org.kylin-music-data.gschema.xml \
    data/org.ukui.log4qt.kylin-music.gschema.xml \
    kylin-music.desktop \
    translations/generate_translations_pm.sh \
    translations/kylin-music_bo_CN.ts \
    translations/kylin-music_zh_CN.ts \
    translations/kylin-music_mn.ts \
	translations/kylin-music_zh_HK.ts

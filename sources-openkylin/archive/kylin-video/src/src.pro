QT += core gui dbus sql KGlobalAccel
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += debug
QMAKE_CXXFLAGS_DEBUG += -g3
QMAKE_LFLAGS_DEBUG += -g3
QMAKE_LFLAGS += -Wl,--gc-sections -Wl,--as-needed
TARGET = kylin-video
CONFIG += c++11 link_pkgconfig
MOC_DIR += .moc
PKGCONFIG += gsettings-qt kysdk-qtwidgets  kysdk-waylandhelper  kysdk-ukuiwindowhelper kysdk-datacollect
contains(CONFIG, KAIMING)  {
    message("KAIMING")
} else {
    message("NOT KAIMING")
    DEFINES += NOTKAIMING
}
TRANSLATIONS += ./translations/kylin-video_zh_CN.ts \
                ./translations/kylin-video_bo_CN.ts \
                ./translations/kylin-video_mn.ts \
                ./translations/kylin-video_de.ts \
                ./translations/kylin-video_es.ts \
                ./translations/kylin-video_fr.ts \
                ./translations/kylin-video_kk.ts \
                ./translations/kylin-video_ug.ts \
                ./translations/kylin-video_ky.ts \
                ./translations/kylin-video_zh_HK.ts \

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS _UNICODE

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += /usr/include/KF5/KWindowSystem/ ./log ./sliderbar ./kwidget /usr/include/kysdk/kysdk-base  /usr/include/KF5/KConfigGui /usr/include

!system($$PWD/translations/generate_translations_pm.sh): error("Failed to generate pm")



contains(CONFIG, KAIMING) {
    target.path = /opt/apps/top.openkylin.kylin-video/files/bin
    target.source = ./kylin-video

    desktop.path = /opt/apps/top.openkylin.kylin-video/files/share/applications
    desktop.files = ../kylin-video.desktop

    qm_files.files = translations/*.qm
    qm_files.path = /opt/apps/top.openkylin.kylin-video/files/share/kylin-video/translations

        help_files.files = ../data/kylin-video/
    help_files.path = /opt/apps/top.openkylin.kylin-video/files/share/kylin-user-guide/data/guide/
} else {
    target.path = /usr/bin
    target.source = ./kylin-video

    desktop.path = /usr/share/applications/
    desktop.files = ../kylin-video.desktop

    qm_files.files = translations/*.qm
    qm_files.path = /usr/share/kylin-video/translations/

        help_files.files = ../data/kylin-video/
    help_files.path = /usr/share/kylin-user-guide/data/guide/
}



INSTALLS += target \
            desktop \
            qm_files \
            help_files

include(core/core.pri)
include(global/global.pri)
include(widget/widget.pri)

LIBS += -lKF5WindowSystem \
        -lmediainfo \
        -lukui-log4qt \
        -lkylog \
 #       -lKF5GuiAddons \
        -lffmpegthumbnailer
 #       -lOpenCL

RESOURCES += \
    resource/res.qrc

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h


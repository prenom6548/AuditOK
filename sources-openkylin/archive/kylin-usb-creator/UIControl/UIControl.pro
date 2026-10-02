QT       += core gui dbus network x11extras KWindowSystem

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 \
    link_pkgconfig
PKGCONFIG += kysdk-datacollect

TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/QRoundProgressBar.cpp \
    src/bottompointlabel.cpp \
    src/dbusadaptor.cpp \
    src/include/aboutdialog.cpp \
    src/include/global.cpp \
    src/include/menumodule.cpp \
    src/include/menufactory.cpp \
    src/include/theme.cpp \
    src/line.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/page1.cpp \
    src/page2.cpp \
    src/include/qtlocalpeer.cpp \
    src/include/qtsingleapplication.cpp \
    src/include/xatom-helper.cpp \
    src/include/daemonipcdbus.cpp \
    src/topborder.cpp

HEADERS += \
    src/QRoundProgressBar.h \
    src/bottompointlabel.h \
    src/dbusadaptor.h \
    src/include/aboutdialog.h \
    src/include/constant_class.h \
    src/include/daemonipcdbus.h \
    src/include/global.h \
    src/include/menumodule.h \
    src/include/menufactory.h \
    src/include/theme.h \
    src/line.h \
    src/mainwindow.h \
    src/page1.h \
    src/page2.h \
    src/include/qtlocalpeer.h \
    src/include/qtlockedfile.h \
    src/include/qtlockedfile.cpp \
    src/include/qtlockedfile_unix.cpp \
    src/include/qtsingleapplication.h \
    src/include/xatom-helper.h \
    src/topborder.h

INCLUDEPATH += \

LIBS +=-lpthread
LIBS +=-lX11
LIBS += -lukui-log4qt

# bin file output dir
TARGET = kylin-usb-creator
target.source += $$TARGET
target.path = /opt/apps/top.openkylin.kylin-usb-creator/files/bin

# 开始菜单/图标主题只扫描 XDG 标准目录，不能只装到 /opt
icons.files = data/kylin-usb-creator.png
icons.path = /usr/share/pixmaps/

# gsettings
schemes.files = data/org.kylin-usb-creator-data.gschema.xml
schemes.path = /usr/share/glib-2.0/schemas/

desktop.path = /usr/share/applications
desktop.files = kylin-usb-creator.desktop

# 开明包内仍保留一份 desktop/icon，供 kaiming 注解 kylin.desktop 使用
opt_desktop.files = kylin-usb-creator.desktop
opt_desktop.path = /opt/apps/top.openkylin.kylin-usb-creator/files/share/applications
opt_icons.files = data/kylin-usb-creator.png
opt_icons.path = /opt/apps/top.openkylin.kylin-usb-creator/files/share/pixmaps/

INSTALLS += target desktop icons schemes opt_desktop opt_icons
CONFIG += link_pkgconfig

PKGCONFIG += gsettings-qt kysdk-waylandhelper kysdk-qtwidgets kysdk-soundeffects kysdk-ukuiwindowhelper
# translations
#qm_files.files = src/translations/*.pm
#qm_files.path = /usr/share/kylin
TRANSLATIONS += src/translations/kylin-usb-creator_zh_CN.ts \
                src/translations/kylin-usb-creator_mn.ts \
                src/translations/kylin-usb-creator_bo_CN.ts \
                src/translations/kylin-usb-creator_zh_HK.ts \
                src/translations/kylin-usb-creator_ug.ts \
                src/translations/kylin-usb-creator_kk.ts \
                src/translations/kylin-usb-creator_vi.ts \
                src/translations/kylin-usb-creator_ms.ts \
                src/translations/kylin-usb-creator_th.ts \
                src/translations/kylin-usb-creator_ky.ts

# Default rules for deployment.
#qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /opt/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target

RESOURCES += data.qrc translations.qrc

QMAKE_CXXFLAGS +=  -Wno-unused-parameter

DISTFILES += src/translations/qt_zh_CN.qm \
    src/translations/kylin-usb-creator_bo_CN.qm \
    src/translations/kylin-usb-creator_bo_CN.ts \
    src/translations/kylin-usb-creator_mn.qm \
    src/translations/kylin-usb-creator_mn.ts \
    src/translations/kylin-usb-creator_mn.qm \
    src/translations/kylin-usb-creator_zh_CN.qm \
    src/translations/kylin-usb-creator_zh_HK.ts \
    src/translations/kylin-usb-creator_zh_HK.qm \
    src/translations/kylin-usb-creator_ug.ts \
    src/translations/kylin-usb-creator_kk.ts \
    src/translations/kylin-usb-creator_ky.ts \
    src/translations/kylin-usb-creator_ug.qm \
    src/translations/kylin-usb-creator_kk.qm \
    src/translations/kylin-usb-creator_ky.qm \
    src/translations/kylin-usb-creator_ms.ts \
    src/translations/kylin-usb-creator_th.ts \
    src/translations/kylin-usb-creator_vi.ts \
    src/translations/kylin-usb-creator_ms.qm \
    src/translations/kylin-usb-creator_th.qm \
    src/translations/kylin-usb-creator_vi.qm

    userM.path = /usr/share/kylin-user-guide/data/guide/kylin-usb-creator/
    userM.files += ../userM/*
    INSTALLS += userM


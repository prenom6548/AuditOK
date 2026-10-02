QT += core gui xml
QT += dbus x11extras KWindowSystem
QT += multimedia

VERSION = 1.3.0.16-0k1.22
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
#CONFIG += no_keywords
CONFIG += c++11
QMAKE_CXXFLAGS += -g
TEMPLATE = app
TARGET = kylin-camera

contains(CONFIG, KAIMING){
   message("kaiming")
   qm_files.path = /opt/apps/top.openkylin.kylin-camera/files
   desktop.path = /opt/apps/top.openkylin.kylin-camera/files/share/applications/
   settings.path = /opt/apps/top.openkylin.kylin-camera/files/share/glib-2.0/schemas/
   guide.path = /opt/apps/top.openkylin.kylin-camera/files/share/kylin-user-guide/data/guide/
   target.path = /opt/apps/top.openkylin.kylin-camera/files/bin
} else {
   message("not kaiming")
   qm_files.path = /usr/share/kylin-camera/translations/
   desktop.path = /usr/share/applications/
   settings.path = /usr/share/glib-2.0/schemas/
   guide.path = /usr/share/kylin-user-guide/data/guide/
   DEFINES += NOTKAIMING
   target.path = /usr/bin/

}

# 配置kysdk
PKGCONFIG += kysdk-kabase kysdk-qtwidgets kysdk-waylandhelper kysdk-ukuiwindowhelper
INCLUDEPATH += /usr/include/kysdk/applications/kabase/

LIBS +=-lpthread
LIBS +=-lX11
CONFIG += link_pkgconfig no_keywords
PKGCONFIG += gsettings-qt gio-2.0 dconf libxml-2.0 dconf libcanberra  libcanberra kysdk-soundeffects
lessThan(QT_MAJOR_VERSION, 4) | lessThan(QT_MINOR_VERSION, 9) {
    message("QT_VERSION ("$$QT_VERSION")")
    DEFINES += __V10__
    QT      -= x11extras
    QT      -= KWindowSystem
    LIBS    -= -lpthread
    LIBS    -= -lX11
}

icons.path = /usr/share/pixmaps/
#target.path = /opt/kylin-camera/bin/
target.source += $$TARGET

settings.files = org.kylin.camera.gschema.xml

schemes.files += \
    $$PWD/data/org.ukui.log4qt.kylin-camera.gschema.xml

guide.files = data/guide/kylin-camera/
   desktop.files = kylin-camera.desktop

INSTALLS += qm_files \
            target \
            desktop \
            settings \
            guide



# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


LIBS += -lkylincamera \
        -L/usr/lib/libukui-log4qt.so.1.0.0 -lukui-log4qt


# 适配窗口管理器圆角阴影


INCLUDEPATH += /usr/include/kylincameralibs/

SOURCES += \
    src/albumread.cpp \
    src/button.cpp \
    src/cameracodestate.cpp \
    src/cameracontrolviewpage.cpp \
    src/camerafilterwidget.cpp \
    src/cameratoppage.cpp \
    src/currentdeviceinfo.cpp \
    src/customstyle.cpp \
    src/defaultgetset.cpp \
    src/delaywdiget.cpp \
    src/devicepull.cpp \
    src/deviceread.cpp \
    src/dirchange.cpp \
    src/dirread.cpp \
    src/globalinstance.cpp \
    src/gridlinepaint.cpp \
    src/hasdevicepage.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/nodevicepage.cpp \
    src/npixcapwidget.cpp \
    src/pictureviewpage.cpp \
    src/recordmodule.cpp \
    src/setting.cpp \
    src/settingpage.cpp \
    src/settingpagetitle.cpp \
    src/stylewidgetattribute.cpp \
    src/switchbutton.cpp \
    src/titlebar.cpp \
    src/util/daemonipcdbus.cpp \
    src/util/login1_dbus.cpp \
    src/util/unix_signals.cpp \
    src/util/xatom-helper.cpp \
    src/util/custombuttonstyle.cpp \
    src/widgetstyle.cpp \
    src/camerakeepalive.cpp \
    src/util/HorizontalOrVerticalMode.cpp \
    src/util/rotatechangeinfo.cpp \
    src/customwidget.cpp


HEADERS += \
    src/albumread.h \
    src/button.h \
    src/cameracodestate.h \
    src/cameracontrolviewpage.h \
    src/camerafilterwidget.h \
    src/cameratoppage.h \
    src/currentdeviceinfo.h \
    src/customstyle.h \
    src/defaultgetset.h \
    src/delaywdiget.h \
    src/devicepull.h \
    src/deviceread.h \
    src/dirchange.h \
    src/dirread.h \
    src/globalinstance.h \
    src/gridlinepaint.h \
    src/hasdevicepage.h \
    src/mainwindow.h \
    src/mylistwidgetitem.h \
    src/nodevicepage.h \
    src/npixcapwidget.h \
    src/pictureviewpage.h \
    src/recordmodule.h \
    src/setting.h \
    src/settingpage.h \
    src/settingpagetitle.h \
    src/stylewidgetattribute.h \
    src/switchbutton.h \
    src/titlebar.h \
    src/util/daemonipcdbus.h \
    src/util/login1_dbus.h \
    src/util/unix_signals.h \
    src/util/xatom-helper.h \
    src/util/custombuttonstyle.h \
    src/widgetstyle.h \
    src/camerakeepalive.h \
    src/util/HorizontalOrVerticalMode.h \ 
    src/util/rotatechangeinfo.h \
    src/customwidget.h

RESOURCES += \
    image.qrc \
    music.qrc

TRANSLATIONS += translations/kylin-camera_zh_CN.ts
TRANSLATIONS += translations/kylin-camera_bo_CN.ts
TRANSLATIONS += translations/kylin-camera_ug.ts
TRANSLATIONS += translations/kylin-camera_ky.ts
TRANSLATIONS += translations/kylin-camera_kk.ts
TRANSLATIONS += translations/kylin-camera_mn.ts
TRANSLATIONS += translations/kylin-camera_zh_HK.ts

!system($$PWD/translations/generate_translations_pm.sh): error("Failed to generate pm")
qm_files.files = translations/*.qm

DISTFILES += \
    data/org.ukui.log4qt.kylin-camera.gschema.xml

FORMS +=






QT       +=  core gui widgets network dbus sql

TEMPLATE = lib
CONFIG += plugin link_pkgconfig c++11 warn_on
PKGCONFIG += gsettings-qt kysdk-qtwidgets kysdk-sysinfo yaml-cpp kysdk-diagnostics kysdk-systime
TARGET = $$qtLibraryTarget(upgrade)
target.path = $$[QT_INSTALL_LIBS]/ukui-control-center/V2.0

MOC_DIR = ./moctmp
UI_DIR = ./uitmp
RCC_DIR = ./rcctmp
OBJECTS_DIR = ./objtmp

TRANSLATIONS += \
    translations/zh_CN.ts \
    translations/tr.ts \
    translations/bo.ts \
    translations/en_US.ts \
    translations/bo_CN.ts \
    translations/mn.ts

QMAKE_CXXFLAGS+=-g
QMAKE_CFLAGS +=-g

# Automating generation .qm files from .ts files
CONFIG(release, debug|release) {
    !system($$PWD/translate_generation.sh): error("Failed to generate translation")
}

isEmpty(PREFIX) {
    PREFIX = /usr
}

qm_files.path = $${PREFIX}/share/upgrade/translations
qm_files.files = translations/*


# 适配窗口管理器圆角阴影
QT += KWindowSystem dbus x11extras

# 适配窗口管理器圆角阴影
LIBS +=-lpthread
LIBS +=-lX11
#LIBS += -lukcc
!contains(QMAKE_HOST.arch, sw_64){
    LIBS += -L$$[QT_INSTALL_LIBS] -lukui-log4qt
}
DEFINES+=KYDEBUG
# 应用图标装载
#icon.path = /usr/share/pixmaps
#icon.files = img/kylin-update-manager.png

# desktop文件装载
#desktop.path = /usr/share/applications/
#desktop.files = ./src/kylin-system-update.desktop

# gsetting文件装载
#schemes.files = data/org.kylin-update-manager-data.gschema.xml
#schemes.path = /usr/share/glib-2.0/schemas/

#config.files = ./config_file/*
#config.path = /usr/share/ukui-control-center/upgrade/

INCLUDEPATH   +=  \
                 $$PROJECT_COMPONENTSOURCE \
                 $$PROJECT_ROOTDIR \
# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS \

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/AddFunction/datetimeutils.cpp \
    src/detaildialog.cpp \
    src/Data/global.cpp \
#    src/logger.cpp \
    src/ukcc/Label/fixlabel.cpp \
    src/ukcc/Label/iconlabel.cpp \
    src/ukcc/Label/lightlabel.cpp \
    src/ukcc/Label/titlelabel.cpp \
    src/ukcc/Label/tristatelabel.cpp \
    src/ukcc/SwitchButton/switchbutton.cpp \
    src/ukcc/interface/upgradeukcccommon.cpp \
    src/xatom-helper.cpp \
    src/appupdate.cpp \
    src/checkbutton.cpp \
    src/daemonipcdbus.cpp \
    src/historyupdatelistwig.cpp \
    src/m_updatelog.cpp \
    src/mylabel.cpp \
#    src/switchbutton.cpp \
    src/tabwidget.cpp \
    src/ukscconn.cpp \
    src/updatedbus.cpp \
    src/updatelog.cpp \
    src/upgrademain.cpp \
    src/SecondaryWidget/setwidget.cpp \
    upgrade.cpp

HEADERS += \
    src/AddFunction/datetimeutils.h \
    src/detaildialog.h \
    src//Data/global.h \
#    src/logger.h \
    src/ukcc/Label/fixlabel.h \
    src/ukcc/Label/iconlabel.h \
    src/ukcc/Label/lightlabel.h \
    src/ukcc/Label/titlelabel.h \
    src/ukcc/Label/tristatelabel.h \
    src/ukcc/SwitchButton/switchbutton.h \
    src/ukcc/interface.h \
    src/ukcc/interface/interface.h \
    src/ukcc/interface/upgradeukcccommon.h \
    src/ukcc/libukcc_global.h \
    src/xatom-helper.h \
    src/appupdate.h \
    src/checkbutton.h \
    src/connection.h \
    src/daemonipcdbus.h \
    src/historyupdatelistwig.h \
    src/m_updatelog.h \
    src/mylabel.h \
#    src/switchbutton.h \
    src/tabwidget.h \
    src/ukscconn.h \
    src/updatedbus.h \
    src/updatelog.h \
    src/upgrademain.h \
    src/utils.h \
    src/SecondaryWidget/setwidget.h \
    upgrade.h


# Default rules for deployment.
INSTALLS += target  \
#            icon    \
#            desktop \
#            schemes \
#            config  \
            qm_files \

RESOURCES += \
    shell/res/img.qrc


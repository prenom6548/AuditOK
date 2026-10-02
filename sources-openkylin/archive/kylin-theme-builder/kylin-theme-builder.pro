QT       += core gui svg KWindowSystem multimedia multimediawidgets concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 link_pkgconfig

PKGCONFIG += kysdk-qtwidgets kysdk-waylandhelper kysdk-sysinfo kysdk-datacollect gsettings-qt

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
    src/build/build.cpp \
    src/build/buildWidget.cpp \
    src/build/buildcheckwidget.cpp \
    src/build/buildresultwidget.cpp \
    src/fileProcess/bridge.cpp \
    src/fileProcess/datacollect.cpp \
    src/fileProcess/diroperation.cpp \
    src/fileProcess/filecheck.cpp \
    src/fileProcess/fileprocess.cpp \
    src/fileProcess/configfilemanager.cpp \
    src/fileProcess/historyinfoload.cpp \
    src/module/aboutdialog.cpp \
    src/module/basewidget.cpp \
    src/module/batchimportdialog.cpp \
    src/module/cacheconfirmedwidget.cpp \
    src/module/cursorthemefeature.cpp \
    src/module/cursorthemewidget.cpp \
    src/module/globalthemefeature.cpp \
    src/module/globalthemewidget.cpp \
    src/module/grubthemefeature.cpp \
    src/module/grubthemewidget.cpp \
    src/module/historywidget.cpp \
    src/module/iconthemewidget.cpp \
    src/module/iconwidgetfeature.cpp \
    src/module/infocreatefeature.cpp \
    src/module/infocreatewidget.cpp \
    src/main.cpp \
    src/maininterface.cpp \
    src/mainwindow.cpp \
    src/module/plymouththemefeature.cpp \
    src/module/plymouththemewidget.cpp \
    src/module/savepathdialog.cpp \
    src/titlebar.cpp

HEADERS += \
    src/build/build.h \
    src/build/buildWidget.h \
    src/build/buildcheckwidget.h \
    src/build/buildresultwidget.h \
    src/fileProcess/bridge.h \
    src/fileProcess/datacollect.h \
    src/fileProcess/diroperation.h \
    src/fileProcess/filecheck.h \
    src/fileProcess/fileprocess.h \
    src/fileProcess/configfilemanager.h \
    src/fileProcess/historyinfoload.h \
    src/module/aboutdialog.h \
    src/module/basewidget.h \
    src/module/batchimportdialog.h \
    src/module/cacheconfirmedwidget.h \
    src/module/cursorthemefeature.h \
    src/module/cursorthemewidget.h \
    src/module/globalthemefeature.h \
    src/module/globalthemewidget.h \
    src/module/grubthemefeature.h \
    src/module/grubthemewidget.h \
    src/module/historywidget.h \
    src/module/iconthemewidget.h \
    src/module/iconwidgetfeature.h \
    src/module/infocreatefeature.h \
    src/module/infocreatewidget.h \
    src/maininterface.h \
    src/mainwindow.h \
    src/module/plymouththemefeature.h \
    src/module/plymouththemewidget.h \
    src/module/savepathdialog.h \
    src/titlebar.h

RESOURCES += \
    resource.qrc

TRANSLATIONS += translation/themebuilder_zh_CN.ts \
                translation/themebuilder_en_US.ts

# Default rules for deployment.
target.source  += $$TARGET
target.path = /usr/bin

qm_files.files = translation/*.qm
qm_files.path = /usr/share/kylin-theme-builder/translations/

appdesktop.files += kylin-theme-builder.desktop
appdesktop.path = /usr/share/applications/

INSTALLS +=target qm_files appdesktop
LIBS += -lukui-log4qt

QT += core gui
QT += dbus xml
QT += network

# 适配窗口管理器圆角阴影
QT += KWindowSystem x11extras

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# 适配窗口管理器圆角阴影
LIBS +=-lpthread
LIBS +=-lX11

# 配置gsettings
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt
PKGCONFIG += kysdk-qtwidgets kysdk-waylandhelper kysdk-soundeffects kysdk-systime kysdk-ukuiwindowhelper

# 配置conf2
PKGCONFIG += kysdk-appconf2

#埋点
PKGCONFIG += kysdk-datacollect
# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
QMAKE_CXXFLAGS += -Wall -ggdb -Wno-implicit-fallthrough

HEADERS += \
    ../backup-daemon/parsebackuplist.h \
    ../common/dynamiccreator.h \
    ../common/mydefine.h \
    ../common/mylittleparse.h \
    ../common/singleton.h \
    ../common/spinlock_mutex.h \
    ../common/utils.h \
    aboutdialog.h \
    backup_manager_interface.h \
    backuppointlistdialog.h \
    component/backuplistwidget.h \
    component/backuppositionselectdialog.h \
    component/circlelabel.h \
    component/clicklabel.h \
    component/doubleclickwidget.h \
    component/filefilterproxymodelforbackup.h \
    component/filepathedit.h \
    component/hoverwidget.h \
    component/imageutil.h \
    component/inputvalidator.h \
    component/linelabel.h \
    component/mycheckbox.h \
    component/myfileselect.h \
    component/myiconbutton.h \
    component/myiconlabel.h \
    component/mylabel.h \
    component/mylineedit.h \
    component/mypushbutton.h \
    component/mywidget.h \
    component/pixmapbutton.h \
    component/pixmaplabel.h \
    component/ringsprogressbar.h \
    datetimeutils.h \
    deletebackupdialog.h \
    functypeconverter.h \
    globalbackupinfo.h \
    globalsignals.h \
    gsettingswrapper.h \
    leftsiderbarwidget.h \
    maindialog.h \
    messageboxutils.h \
    module/databackup.h \
    module/datarestore.h \
    module/ghostimage.h \
    module/managebackuppointlist.h \
    module/operationlog.h \
    module/selectrestorepoint.h \
    module/systembackup.h \
    module/systemrestore.h \
    module/udiskdetector.h \
    qtsingleapplication/qtlocalpeer.h \
    qtsingleapplication/qtlockedfile.h \
    qtsingleapplication/qtsingleapplication.h \
    xatom-helper.h

SOURCES += \
    ../backup-daemon/parsebackuplist.cpp \
    ../common/mydefine.cpp \
    ../common/mylittleparse.cpp \
    ../common/utils.cpp \
    aboutdialog.cpp \
    backup_manager_interface.cpp \
    backuppointlistdialog.cpp \
    component/backuplistwidget.cpp \
    component/backuppositionselectdialog.cpp \
    component/circlelabel.cpp \
    component/clicklabel.cpp \
    component/doubleclickwidget.cpp \
    component/filefilterproxymodelforbackup.cpp \
    component/filepathedit.cpp \
    component/hoverwidget.cpp \
    component/imageutil.cpp \
    component/inputvalidator.cpp \
    component/linelabel.cpp \
    component/mycheckbox.cpp \
    component/myfileselect.cpp \
    component/myiconbutton.cpp \
    component/myiconlabel.cpp \
    component/mylabel.cpp \
    component/mylineedit.cpp \
    component/mypushbutton.cpp \
    component/mywidget.cpp \
    component/pixmapbutton.cpp \
    component/pixmaplabel.cpp \
    component/ringsprogressbar.cpp \
    datetimeutils.cpp \
    deletebackupdialog.cpp \
    functypeconverter.cpp \
    gsettingswrapper.cpp \
    leftsiderbarwidget.cpp \
    main.cpp \
    maindialog.cpp \
    messageboxutils.cpp \
    module/databackup.cpp \
    module/datarestore.cpp \
    module/ghostimage.cpp \
    module/managebackuppointlist.cpp \
    module/operationlog.cpp \
    module/selectrestorepoint.cpp \
    module/systembackup.cpp \
    module/systemrestore.cpp \
    module/udiskdetector.cpp \
    qtsingleapplication/qtlocalpeer.cpp \
    qtsingleapplication/qtlockedfile.cpp \
    qtsingleapplication/qtlockedfile_unix.cpp \
    qtsingleapplication/qtsingleapplication.cpp \
    xatom-helper.cpp

FORMS += \
    aboutdialog.ui \
    backuppointlistdialog.ui \
    deletebackupdialog.ui \
    maindialog.ui

OTHER_FILES +=

TRANSLATIONS += qt_zh_CN.ts \
                qt_bo_CN.ts \
                qt_mn_MN.ts \
                qt_zh_HK.ts \
                qt_kk.ts    \
                qt_ky.ts    \
                qt_ug.ts    \
                qt_ms.ts    \
                qt_vi.ts    \
                qt_th.ts

message($$system(./generate_translations_qm.sh))
#system(rm -f qt_zh_CN.qm)
#!system(lrelease qt_zh_CN.ts): error("Failed to generate cp qt_zh_CN.qm")
#system(cp qt_zh_CN.qm resource/language/)
#system(rm -f qt_bo_CN.qm)
#!system(lrelease qt_bo_CN.ts): error("Failed to generate qt_bo_CN.qm")
#system(cp qt_bo_CN.qm resource/language/)
#system(rm -f qt_mn_MN.qm)
#!system(lrelease qt_mn_MN.ts): error("Failed to generate qt_mn_MN.qm")
#system(cp qt_mn_MN.qm resource/language/)

RESOURCES += \
    app.qrc


# Default rules for deployment.
#qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /opt/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target

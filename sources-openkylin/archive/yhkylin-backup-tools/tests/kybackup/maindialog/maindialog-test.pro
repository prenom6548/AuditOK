TEMPLATE = app
TARGET = test_maindialog

# Qt modules
QT += core gui widgets testlib dbus xml
QT += KWindowSystem x11extras
QT += network

# C++ standard
CONFIG += c++11

# 配置gsettings
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt
PKGCONFIG += kysdk-qtwidgets kysdk-waylandhelper kysdk-soundeffects kysdk-systime kysdk-ukuiwindowhelper
PKGCONFIG += kysdk-appconf2
PKGCONFIG += kysdk-datacollect

# Include paths
INCLUDEPATH += $$PWD/../../../kybackup
INCLUDEPATH += $$PWD/../../../kybackup/component
INCLUDEPATH += $$PWD/../../../kybackup/module
INCLUDEPATH += $$PWD/../../../common
INCLUDEPATH += $$PWD/../../../backup-daemon

# Source files - 只包含测试相关的源文件
SOURCES += \
    test_maindialog.cpp \
    ../../../kybackup/maindialog.cpp \
    ../../../kybackup/leftsiderbarwidget.cpp \
    ../../../kybackup/component/doubleclickwidget.cpp \
    ../../../kybackup/component/mywidget.cpp \
    ../../../kybackup/xatom-helper.cpp \
    ../../../kybackup/aboutdialog.cpp \
    ../../../kybackup/backup_manager_interface.cpp \
    ../../../kybackup/backuppointlistdialog.cpp \
    ../../../kybackup/deletebackupdialog.cpp \
    ../../../kybackup/functypeconverter.cpp \
    ../../../kybackup/gsettingswrapper.cpp \
    ../../../kybackup/messageboxutils.cpp \
    ../../../kybackup/component/backuplistwidget.cpp \
    ../../../kybackup/component/backuppositionselectdialog.cpp \
    ../../../kybackup/component/circlelabel.cpp \
    ../../../kybackup/component/clicklabel.cpp \
    ../../../kybackup/component/linelabel.cpp \
    ../../../kybackup/component/filefilterproxymodelforbackup.cpp \
    ../../../kybackup/component/filepathedit.cpp \
    ../../../kybackup/component/hoverwidget.cpp \
    ../../../kybackup/component/imageutil.cpp \
    ../../../kybackup/component/inputvalidator.cpp \
    ../../../kybackup/component/mycheckbox.cpp \
    ../../../kybackup/component/myfileselect.cpp \
    ../../../kybackup/component/myiconbutton.cpp \
    ../../../kybackup/component/myiconlabel.cpp \
    ../../../kybackup/component/mylabel.cpp \
    ../../../kybackup/component/mylineedit.cpp \
    ../../../kybackup/component/mypushbutton.cpp \
    ../../../kybackup/component/pixmapbutton.cpp \
    ../../../kybackup/component/pixmaplabel.cpp \
    ../../../kybackup/component/ringsprogressbar.cpp \
    ../../../kybackup/datetimeutils.cpp \
    ../../../kybackup/module/databackup.cpp \
    ../../../kybackup/module/datarestore.cpp \
    ../../../kybackup/module/ghostimage.cpp \
    ../../../kybackup/module/managebackuppointlist.cpp \
    ../../../kybackup/module/operationlog.cpp \
    ../../../kybackup/module/selectrestorepoint.cpp \
    ../../../kybackup/module/systembackup.cpp \
    ../../../kybackup/module/systemrestore.cpp \
    ../../../kybackup/module/udiskdetector.cpp \
    ../../../kybackup/qtsingleapplication/qtlocalpeer.cpp \
    ../../../kybackup/qtsingleapplication/qtlockedfile.cpp \
    ../../../kybackup/qtsingleapplication/qtlockedfile_unix.cpp \
    ../../../kybackup/qtsingleapplication/qtsingleapplication.cpp \
    ../../../common/mydefine.cpp \
    ../../../common/mylittleparse.cpp \
    ../../../common/utils.cpp \
    ../../../backup-daemon/parsebackuplist.cpp

# Header files
HEADERS += \
    test_maindialog.h \
    ../../../kybackup/maindialog.h \
    ../../../kybackup/leftsiderbarwidget.h \
    ../../../kybackup/component/doubleclickwidget.h \
    ../../../kybackup/component/mywidget.h \
    ../../../kybackup/globalbackupinfo.h \
    ../../../kybackup/globalsignals.h \
    ../../../kybackup/xatom-helper.h \
    ../../../kybackup/aboutdialog.h \
    ../../../kybackup/backup_manager_interface.h \
    ../../../kybackup/backuppointlistdialog.h \
    ../../../kybackup/deletebackupdialog.h \
    ../../../kybackup/functypeconverter.h \
    ../../../kybackup/gsettingswrapper.h \
    ../../../kybackup/messageboxutils.h \
    ../../../kybackup/component/backuplistwidget.h \
    ../../../kybackup/component/backuppositionselectdialog.h \
    ../../../kybackup/component/circlelabel.h \
    ../../../kybackup/component/clicklabel.h \
    ../../../kybackup/component/linelabel.h \
    ../../../kybackup/component/filefilterproxymodelforbackup.h \
    ../../../kybackup/component/filepathedit.h \
    ../../../kybackup/component/hoverwidget.h \
    ../../../kybackup/component/imageutil.h \
    ../../../kybackup/component/inputvalidator.h \
    ../../../kybackup/component/mycheckbox.h \
    ../../../kybackup/component/myfileselect.h \
    ../../../kybackup/component/myiconbutton.h \
    ../../../kybackup/component/myiconlabel.h \
    ../../../kybackup/component/mylabel.h \
    ../../../kybackup/component/mylineedit.h \
    ../../../kybackup/component/mypushbutton.h \
    ../../../kybackup/component/pixmapbutton.h \
    ../../../kybackup/component/pixmaplabel.h \
    ../../../kybackup/component/ringsprogressbar.h \
    ../../../kybackup/datetimeutils.h \
    ../../../kybackup/module/databackup.h \
    ../../../kybackup/module/datarestore.h \
    ../../../kybackup/module/ghostimage.h \
    ../../../kybackup/module/managebackuppointlist.h \
    ../../../kybackup/module/operationlog.h \
    ../../../kybackup/module/selectrestorepoint.h \
    ../../../kybackup/module/systembackup.h \
    ../../../kybackup/module/systemrestore.h \
    ../../../kybackup/module/udiskdetector.h \
    ../../../kybackup/qtsingleapplication/qtlocalpeer.h \
    ../../../kybackup/qtsingleapplication/qtlockedfile.h \
    ../../../kybackup/qtsingleapplication/qtsingleapplication.h \
    ../../../common/dynamiccreator.h \
    ../../../common/mydefine.h \
    ../../../common/mylittleparse.h \
    ../../../common/singleton.h \
    ../../../common/spinlock_mutex.h \
    ../../../common/utils.h \
    ../../../backup-daemon/parsebackuplist.h

# # Form files (if any)
FORMS += \
    ../../../kybackup/maindialog.ui \
    ../../../kybackup/aboutdialog.ui \
    ../../../kybackup/backuppointlistdialog.ui \
    ../../../kybackup/deletebackupdialog.ui

# # Resource files (if any)
# RESOURCES += \
#     ../../../kybackup/app.qrc

# Coverage flags
QMAKE_CXXFLAGS += --coverage
QMAKE_LFLAGS += --coverage -lgcov

# Define any necessary macros
DEFINES += UNIT_TEST

# Check if this is a library and add library linking if needed
# For now, assuming it's not a library as per the input parameter

# Link libraries
LIBS += -lpthread -lX11
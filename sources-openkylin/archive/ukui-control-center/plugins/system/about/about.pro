include(../../../env.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/Label/label.pri)
include($$PROJECT_ROOTDIR/libukcc/interface.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/SettingWidget/settingwidget.pri)

QT       += widgets dbus concurrent svg
TEMPLATE = lib
CONFIG += plugin \
          link_pkgconfig

PKGCONFIG     += gsettings-qt \
                 kysdk-qtwidgets \
                 kysdk-sysinfo \
                 kysdk-datacollect \
                 kysdk-hardware \

TARGET = $$qtLibraryTarget(about)
DESTDIR = ../..
target.path = $${PLUGIN_INSTALL_DIRS}

protocol.files += res/*.txt
protocol.path = /usr/share/protocol

INCLUDEPATH   +=  \
                 $$PROJECT_ROOTDIR \


if(!contains(DEFINES, OPENKYLIN)) {
LIBS += -L$$[QT_INSTALL_LIBS] -lkimclient
}


HEADERS += \
    HpQRCodeInterface.h \
    about.h \
    aboutui.h \
    hostnamedialog.h \
    privacydialog.h \
    statusdialog.h \
    trialdialog.h

SOURCES += \
    about.cpp \
    aboutui.cpp \
    hostnamedialog.cpp \
    privacydialog.cpp \
    statusdialog.cpp \
    trialdialog.cpp

FORMS +=

RESOURCES += \
    res/img.qrc

INSTALLS += target \
            protocol

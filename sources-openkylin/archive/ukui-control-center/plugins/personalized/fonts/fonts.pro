#-------------------------------------------------
#
# Project created by QtCreator 2019-06-25T12:51:26
#
#-------------------------------------------------

include(../../../env.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/Label/label.pri)
include($$PROJECT_ROOTDIR/libukcc/interface.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/FlowLayout/flowlayout.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/SettingWidget/settingwidget.pri)

QT       += widgets dbus KWindowSystem

TEMPLATE = lib
CONFIG   += plugin

TARGET = $$qtLibraryTarget(fonts)
DESTDIR = ../..
target.path = $${PLUGIN_INSTALL_DIRS}
INSTALLS += target

##加载gio库和gio-unix库，用于获取gsettings的默认值
CONFIG        += link_pkgconfig \
                 C++11
PKGCONFIG     += gio-2.0 \
                 gio-unix-2.0 \
                 gsettings-qt \
                 kysdk-qtwidgets \
                 kysdk-ukenv \
                 freetype2 \
                 fontconfig \

INCLUDEPATH   +=  \
                 $$PROJECT_ROOTDIR \
                 kabase/ \
                 kabase/Qt

LIBS += -L$$[QT_INSTALL_LIBS] -lgsettings-qt -lfontconfig -lfreetype

#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        blankpage.cpp \
        fonts.cpp \
        fontunit.cpp \
        libfun.cpp \
        popupfontinfo.cpp \
        ../../../shell/pinyin.cpp \

HEADERS += \
        blankpage.h \
        fonts.h \
        fontunit.h \
        libfun.h \
        popupfontinfo.h \
        ../../../shell/pinyin.h \

FORMS += \
        fonts.ui

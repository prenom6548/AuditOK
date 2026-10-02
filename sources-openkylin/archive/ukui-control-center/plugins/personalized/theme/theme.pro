#-------------------------------------------------
#
# Project created by QtCreator 2019-06-21T10:50:19
#
#-------------------------------------------------
include(../../../env.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/FlowLayout/flowlayout.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/Label/label.pri)
include($$PROJECT_ROOTDIR/libukcc/interface.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/SettingWidget/settingwidget.pri)

QT       += widgets KConfigCore KI18n x11extras concurrent dbus

TEMPLATE = lib
CONFIG += plugin \
          link_pkgconfig \
          c++11
TARGET = $$qtLibraryTarget(theme)
DESTDIR = ../..
target.path = $${PLUGIN_INSTALL_DIRS}
INSTALLS += target

INCLUDEPATH   += $$PROJECT_ROOTDIR \

LIBS          += -L$$[QT_INSTALL_LIBS] -lgsettings-qt -lX11 -lXfixes -lXcursor

PKGCONFIG     += gsettings-qt \
                 kysdk-qtwidgets \
                 kysdk-sysinfo \
                 kysdk-package \

SOURCES += \
    cursor/cursortheme.cpp \
    cursor/xcursortheme.cpp \
    drawiconlabel.cpp \
    globaltheme/customglobaltheme.cpp \
    globaltheme/globaltheme.cpp \
    globaltheme/globalthemehelper.cpp \
    icontheme.cpp \
    myqradiobutton.cpp \
    myslider.cpp \
    radioproxystyle.cpp \
    theme.cpp \
    themebutton.cpp \
    themeiconlabel.cpp \
    themewidget.cpp \
    widgetgroup.cpp \
    ../../../shell/customstyle.cpp

HEADERS += \
    cursor/config-X11.h \
    cursor/cursortheme.h \
    cursor/xcursortheme.h \
    drawiconlabel.h \
    globaltheme/customglobaltheme.h \
    globaltheme/globaltheme.h \
    globaltheme/globaltheme_p.h \
    globaltheme/globalthemehelper.h \
    globaltheme/globalthemehelper_p.h \
    icontheme.h \
    myqradiobutton.h \
    myslider.h \
    radioproxystyle.h \
    theme.h \
    themebutton.h \
    themeiconlabel.h \
    themewidget.h \
    widgetgroup.h \
    ../../../shell/customstyle.h

FORMS += \
    theme.ui

#-------------------------------------------------
#
# Project created by QtCreator 2019-02-20T15:36:43
#
#-------------------------------------------------
include(../../../env.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/Label/label.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/ComboBox/combobox.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/SettingWidget/settingwidget.pri)
include($$PROJECT_ROOTDIR/libukcc/interface.pri)

QT            += widgets core gui quickwidgets quick xml KScreen KWindowSystem dbus concurrent
TEMPLATE = lib
CONFIG        += c++11   link_pkgconfig plugin

TARGET = $$qtLibraryTarget(display)
DESTDIR = ../..
target.path = $${PLUGIN_INSTALL_DIRS}
INSTALLS += target
#DEFINES += AUTOBRIHTNESS
INCLUDEPATH   +=  $$PROJECT_ROOTDIR \

LIBS          += -L$$[QT_INSTALL_LIBS] -lgsettings-qt

PKGCONFIG += gsettings-qt     \
             kysdk-qtwidgets \
             kysdk-sysinfo \
             kscreen2 \
             kysdk-datacollect \
             kysdk-waylandhelper \

SOURCES += \
    brightnessFrame.cpp \
    display.cpp \
    declarative/qmloutput.cpp \
    declarative/qmloutputcomponent.cpp \
    declarative/qmlscreen.cpp \
    controlpanel.cpp \
    getBrightnessThread.cpp \
    outputconfig.cpp \
    resolutionslider.cpp \
    unifiedoutputconfig.cpp \
    utils.cpp \
    widget.cpp \
    utilsofdisplay.cpp

HEADERS += \
    brightnessFrame.h \
    colorinfo.h \
    display.h \
    declarative/qmloutput.h \
    declarative/qmloutputcomponent.h \
    declarative/qmlscreen.h \
    controlpanel.h \
    getBrightnessThread.h \
    outputconfig.h \
    resolutionslider.h \
    scalesize.h \
    unifiedoutputconfig.h \
    utils.h \
    widget.h \
    utilsofdisplay.h

FORMS += \
    display.ui \
#DISTFILES += \
#    qml/main.qml \
#    qml/Output.qml

RESOURCES += \
    qml.qrc

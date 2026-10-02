#-------------------------------------------------
#
# Project created by QtCreator 2019-06-29T15:08:41
#
#-------------------------------------------------

QT       += widgets dbus x11extras svg

TEMPLATE = lib
CONFIG += plugin \
          link_pkgconfig \
          C++11

include(../../../env.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/FlowLayout/flowlayout.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/Label/label.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/AddBtn/addbtn.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/HoverWidget/hoverwidget.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/Frame/frame.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/SettingWidget/settingwidget.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/Button/button.pri)

include($$PROJECT_ROOTDIR/libukcc/interface.pri)

TARGET = $$qtLibraryTarget(datetime)
DESTDIR = ../..
target.path = $${PLUGIN_INSTALL_DIRS}
INSTALLS += target

INCLUDEPATH   +=  \
                 $$PROJECT_ROOTDIR \

LIBS += -L$$[QT_INSTALL_LIBS] -lgsettings-qt -lpolkit-qt5-core-1

##加载gio库和gio-unix库，用于处理时间
PKGCONFIG     += gio-2.0 \
                 gio-unix-2.0 \
                 gsettings-qt \
                 kysdk-qtwidgets \
                 kysdk-datacollect \
                 kysdk-systime

#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    calendarwidget.cpp \
    clockwidget.cpp \
    dateedit.cpp \
    dategroupwidget.cpp \
    datetime.cpp \
    timeBtn.cpp \
    datetimeui.cpp \
    timelabel.cpp \
    timezonechooserdialog.cpp \
    worldMap/zoneinfo.cpp \

HEADERS += \
    calendarwidget.h \
    clockwidget.h \
    dateedit.h \
    dategroupwidget.h \
    datetime.h \
    timeBtn.h \
    datetimeui.h \
    timelabel.h \
    timezonechooserdialog.h \
    worldMap/zoneinfo.h \

FORMS +=

RESOURCES += \
    tz.qrc

DISTFILES +=

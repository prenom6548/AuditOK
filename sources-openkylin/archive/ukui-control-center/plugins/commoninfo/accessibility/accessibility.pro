#-------------------------------------------------
#
# Project created by QtCreator 2019-02-20T15:36:43
#
#-------------------------------------------------
include(../../../env.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/Label/label.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/Button/button.pri)
include($$PROJECT_ROOTDIR/libukcc/interface.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/SettingWidget/settingwidget.pri)

QT            += widgets gui dbus KConfigCore KI18n
TEMPLATE = lib
CONFIG        += c++11 link_pkgconfig plugin

TARGET = $$qtLibraryTarget(accessibility)
DESTDIR = ../..
target.path = $${PLUGIN_INSTALL_DIRS}
INSTALLS += target
INCLUDEPATH   +=  $$PROJECT_ROOTDIR

LIBS          += -L$$[QT_INSTALL_LIBS] -lgsettings-qt

PKGCONFIG += gsettings-qt     \
             kysdk-qtwidgets

SOURCES += \
        accessibility.cpp \

HEADERS += \
        accessibility.h \
        accessibilityui.h

RESOURCES += \
	acc.qrc

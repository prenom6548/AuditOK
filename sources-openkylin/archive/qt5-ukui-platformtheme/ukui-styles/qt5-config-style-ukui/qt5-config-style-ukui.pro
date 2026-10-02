#-------------------------------------------------
#
# Project created by QtCreator 2020-01-03T19:31:51
#
#-------------------------------------------------

QT       += widgets widgets-private KWindowSystem gui gui-private x11extras dbus KWaylandClient

TARGET = qt5-config-style-ukui
TEMPLATE = lib
CONFIG += plugin c++11 link_pkgconfig
PKGCONFIG += gsettings-qt

include(../../libqt5-ukui-style/libqt5-ukui-style.pri)
include(animations/animations.pri)
include(widget-parameters/widget-parameters.pri)

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
#DEFINES += QT_NO_DEBUG_OUTPUT
DEFINES += QT_MESSAGELOGCONTEXT

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ukui-config-style-parameters.cpp \
    #ukui-config-style-plugin.cpp \
    ukui-config-style.cpp \
    shadow-helper.cpp \
    ukui-config-style-helper.cpp \
    readconfig.cpp \
    view-helper.cpp \
    ../common/blur-helper.cpp \
    ../common/gesture-helper.cpp \
    ../common/window-manager.cpp \
    ../common/ukui-style-plugin.cpp


HEADERS += \
    ukui-config-style-parameters.h \
    #ukui-config-style-plugin.h \
    ukui-config-style.h \
    shadow-helper.h \
    ukui-config-style-helper.h \
    readconfig.h \
    ../common/blur-helper.h \
    ../common/gesture-helper.h \
    ../common/window-manager.h \
    ../common/ukui-style-plugin.h

DISTFILES += ../common/qt5-style-ukui.json \
    themeconfig/UKUIConfigFashion.json \
    themeconfig/UKUIConfigDefault.json

unix {
    target.path = $$[QT_INSTALL_PLUGINS]/styles
    INSTALLS += target
}

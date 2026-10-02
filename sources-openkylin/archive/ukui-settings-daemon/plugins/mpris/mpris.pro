#-------------------------------------------------
#
# Project created by QtCreator 2020-06-24T09:30:00
#
#-------------------------------------------------
QT -= gui
QT += dbus
TEMPLATE = lib
TARGET = mpris

CONFIG += c++11 plugin link_pkgconfig
CONFIG -= app_bundle
DEFINES += QT_DEPRECATED_WARNINGS MODULE_NAME=\\\"mpris\\\"

include($$PWD/../../common/common.pri)

SOURCES += \
    dbus_properties.cpp \
    mpris_controller.cpp \
    mpris_controller_adaptor.cpp \
    mpris_player.cpp \
    mpris_player_control.cpp \
    mpris_player_manager.cpp \
    mpris_plugin.cpp \
    org_mpris_MediaPlayer2.cpp \
    org_mpris_MediaPlayer2_Player.cpp

HEADERS += \
    dbus_properties.h \
    mpris-plugin.h \
    mpris_controller.h \
    mpris_controller_adaptor.h \
    mpris_player.h \
    mpris_player_control.h \
    mpris_player_manager.h \
    org_mpris_MediaPlayer2.h \
    org_mpris_MediaPlayer2_Player.h

mpris_lib.path = $${PLUGIN_INSTALL_DIRS}
mpris_lib.files = $$OUT_PWD/libmpris.so

INSTALLS += mpris_lib

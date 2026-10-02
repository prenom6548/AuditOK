#-------------------------------------------------
#
# Project created by QtCreator 2020-06-16T09:30:00
#
#-------------------------------------------------
QT += gui widgets dbus KGlobalAccel KWindowSystem
TEMPLATE = lib
CONFIG += c++11 plugin link_pkgconfig
CONFIG -= app_bundle

PKGCONFIG += \
    kysdk-waylandhelper \
    kysdk-ukuiwindowhelper \

LIBS += \
    -lX11 -lXi -lpulse

include($$PWD/../../common/common.pri)

DEFINES += QT_DEPRECATED_WARNINGS HAVE_X11_EXTENSIONS_XKB_H _FORTIFY_SOURCE=2 MODULE_NAME=\\\"mediakeys\\\"
QMAKE_CFLAGS_DEBUG -= O2
QMAKE_CXXFLAGS_DEBUG -= -O2

QMAKE_CFLAGS_RELEASE -= O2
QMAKE_CXXFLAGS_RELEASE -= -O2

SOURCES += \
    media-action-settings.cpp \
    media-key-block-shutcut.cpp \
    media-key-action.cpp \
    media-key-binding.cpp \
    media-key-cancel.cpp \
    media-key-manager.cpp \
    media-key-plugin.cpp \
    media-key-settings-abstract.cpp \
    media-key-settings-extend.cpp \
    media-key-settings.cpp \
    rfkill-state.cpp \
    sound.cpp \
    widget/pop-window-helper.cpp \
    widget/privacy-osd.cpp  \
    xEventMonitor.cpp

HEADERS += \
    media-action-settings.h \
    media-key-block-shutcut.h \
    media-key-action.h \
    media-key-binding.h \
    media-key-cancel.h \
    media-key-common.h \
    media-key-manager.h \
    media-key-plugin.h \
    media-key-settings-abstract.h \
    media-key-settings-extend.h \
    media-key-settings.h \
    media-type.h \
    rfkill-state.h \
    sound.h \
    transform_key.h \
    widget/pop-window-helper.h \
    widget/privacy-osd.h  \
    xEventMonitor.h


DISTFILES += \
    media-keys.ukui-settings-plugin.in

media_keys_lib.path = $${PLUGIN_INSTALL_DIRS}
media_keys_lib.files = $$OUT_PWD/libmedia-keys.so

INSTALLS += media_keys_lib

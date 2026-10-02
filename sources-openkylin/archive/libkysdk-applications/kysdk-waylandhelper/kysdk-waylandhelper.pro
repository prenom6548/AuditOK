QT += KWaylandClient KWaylandServer gui widgets x11extras KWindowSystem KIconThemes concurrent

TEMPLATE = lib
DEFINES += KYSDKWAYLANDHELPER_LIBRARY

CONFIG += c++11 link_pkgconfig
PKGCONFIG += wayland-client x11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += USE_UKUI_PROTOCOL
DEFINES += USE_UKUI_SHELL_PLUGIN
DEFINES += DEPERCATED_OLD_API

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

system(wayland-scanner client-header protocols/ukui-shell-v1.xml $$OUT_PWD/ukui-shell-client-protocol.h)
system(wayland-scanner private-code protocols/ukui-shell-v1.xml $$OUT_PWD/ukui-shell-core-protocol.c)

system(wayland-scanner client-header protocols/ukui-window-management.xml $$OUT_PWD/ukui-window-management-client-protocol.h)
system(wayland-scanner private-code protocols/ukui-window-management.xml $$OUT_PWD/ukui-window-management-core-protocol.c)

system(wayland-scanner client-header protocols/xdg-activation-v1.xml $$OUT_PWD/wayland-xdg-activation-v1-client-protocol.h)
system(wayland-scanner private-code protocols/xdg-activation-v1.xml $$OUT_PWD/wayland-xdg-activation-v1-protocol.c)

system(wayland-scanner client-header protocols/ext-idle-notify-v1.xml $$OUT_PWD/wayland-ext-idle-notify-v1-client-protocol.h)
system(wayland-scanner private-code protocols/ext-idle-notify-v1.xml $$OUT_PWD/wayland-ext-idle-notify-v1-protocol.c)

system(wayland-scanner client-header protocols/ukui-blur-v1.xml $$OUT_PWD/ukui-blur-manager-client-protocol.h)
system(wayland-scanner private-code protocols/ukui-blur-v1.xml $$OUT_PWD/ukui-blur-manager-core-protocol.c)

system(wayland-scanner client-header protocols/plasma-window-management.xml $$OUT_PWD/wayland-plasma-window-management-client-protocol.h)
system(wayland-scanner private-code protocols/plasma-window-management.xml $$OUT_PWD/wayland-plasma-window-management-protocol.c)


SOURCES += \
    src/ukuistylehelper/ukui-decoration-core.c \
    src/ukuistylehelper/ukui-decoration-manager.cpp \
    src/ukuistylehelper/ukuistylehelper.cpp \
    src/ukuistylehelper/xatom-helper.cpp \
    src/ukuistylehelper/ukui-shell-manager.cpp \
    src/waylandhelper.cpp \
    src/windowmanager/abstractinterface.cpp \
    src/windowmanager/kdewaylandinterface.cpp \
    src/windowmanager/outputinfo.cpp \
    src/windowmanager/ukuiwaylandinterface.cpp \
    src/windowmanager/xcbinterface.cpp \
    src/windowmanager/ukuiwindowmanagement.cpp \
    src/windowmanager/wmregister.cpp \
    src/windowmanager/windowmanager.cpp \
    src/windowmanager/extidlenotifier.cpp \
    src/windowmanager/idlenotifier.cpp \
    src/windowmanager/ukuishell.cpp \
    src/windowmanager/xdgactivation.cpp \
    src/blurmanager/ukuiblur.cpp \
    src/blurmanager/blurmanager.cpp \
    src/blurmanager/blurwaylandinterface.cpp \
    src/blurmanager/blurxcbinterface.cpp \
    $$OUT_PWD/ukui-blur-manager-core-protocol.c \
    $$OUT_PWD/ukui-window-management-core-protocol.c \
    $$OUT_PWD/ukui-shell-core-protocol.c \
    $$OUT_PWD/wayland-xdg-activation-v1-protocol.c \
    $$OUT_PWD/wayland-ext-idle-notify-v1-protocol.c \
    $$OUT_PWD/wayland-plasma-window-management-protocol.c \


HEADERS += \
    src/ukuistylehelper/ukui-decoration-client.h \
    src/ukuistylehelper/ukui-decoration-manager.h \
    src/ukuistylehelper/ukuistylehelper.h \
    src/ukuistylehelper/xatom-helper.h \
    src/ukuistylehelper/ukui-shell-manager.h \
    src/waylandhelper.h \
    src/windowmanager/abstractinterface.h \
    src/windowmanager/kdewaylandinterface.h \
    src/windowmanager/outputinfo.h \
    src/windowmanager/xcbinterface.h \
    src/windowmanager/ukuiwaylandinterface.h \
    src/windowmanager/ukuiwaylandpointer.h \
    src/windowmanager/ukuiwindowmanagement.h \
    src/windowmanager/windowinfo.h \
    src/windowmanager/wmregister.h \
    src/windowmanager/windowmanager.h \
    src/windowmanager/xdgactivation.h \
    src/windowmanager/ukuishell.h \
    src/windowmanager/extidlenotifier.h \
    src/windowmanager/idlenotifier.h \
    src/windowmanager/kyatoms_p.h \
    src/blurmanager/ukuiblur.h \
    src/blurmanager/blurmanager.h \
    src/blurmanager/blurabstractinterface.h \
    src/blurmanager/blurwaylandinterface.h \
    src/blurmanager/blurxcbinterface.h \
    $$OUT_PWD/ukui-blur-manager-client-protocol.h\
    $$OUT_PWD/ukui-shell-client-protocol.h \
    $$OUT_PWD/wayland-plasma-window-management-client-protocol.h \
    $$OUT_PWD/ukui-window-management-client-protocol.h \
    $$OUT_PWD/wayland-xdg-activation-v1-client-protocol.h \
    $$OUT_PWD/wayland-ext-idle-notify-v1-client-protocol.h \

INCLUDEPATH+=src \
    src/windowmanager \
    $$OUT_PWD \

# Default rules for deployment.
headers.files = src/kysdk-waylandhelper_global.h \
                src/waylandhelper.h
headers.path = /usr/include/kysdk/applications/

wm_headers.files = src/windowmanager/abstractinterface.h \
    src/windowmanager/kdewaylandinterface.h \
    src/windowmanager/xcbinterface.h \
    src/windowmanager/ukuiwaylandinterface.h \
    src/windowmanager/ukuiwaylandpointer.h \
    src/windowmanager/ukuiwindowmanagement.h \
    src/windowmanager/windowinfo.h \
    src/windowmanager/wmregister.h \
    src/windowmanager/windowmanager.h \
    src/windowmanager/xdgactivation.h \
    src/windowmanager/ukuishell.h \
    src/windowmanager/extidlenotifier.h \
    src/windowmanager/idlenotifier.h \
    src/windowmanager/kyatoms_p.h \
    src/windowmanager/outputinfo.h \
    $$OUT_PWD/ukui-shell-client-protocol.h \
    $$OUT_PWD/wayland-plasma-window-management-client-protocol.h \
    $$OUT_PWD/ukui-window-management-client-protocol.h \
    $$OUT_PWD/wayland-xdg-activation-v1-client-protocol.h \
    $$OUT_PWD/wayland-ext-idle-notify-v1-client-protocol.h \
                    
wm_headers.path = /usr/include/kysdk/applications/windowmanager/

sty_headers.files = src/ukuistylehelper/ukui-decoration-client.h \
                    src/ukuistylehelper/ukui-decoration-manager.h \
                    src/ukuistylehelper/ukuistylehelper.h \
                    src/ukuistylehelper/xatom-helper.h \
                    src/ukuistylehelper/ukui-shell-manager.h


sty_headers.path = /usr/include/kysdk/applications/ukuistylehelper/

blur_headers.files = src/blurmanager/ukuiblur.h \
                    src/blurmanager/blurmanager.h \
                    src/blurmanager/blurabstractinterface.h \
                    src/blurmanager/blurwaylandinterface.h \
                    src/blurmanager/blurxcbinterface.h \
                    $$OUT_PWD/ukui-blur-manager-client-protocol.h \

blur_headers.path = /usr/include/kysdk/applications/blurmanager/

target.path = $$[QT_INSTALL_LIBS]
INSTALLS += target headers wm_headers sty_headers blur_headers

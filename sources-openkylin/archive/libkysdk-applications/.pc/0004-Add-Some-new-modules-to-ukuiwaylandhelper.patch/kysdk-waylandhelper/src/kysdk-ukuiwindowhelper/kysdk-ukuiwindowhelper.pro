QT += gui widgets

TEMPLATE = lib

CONFIG += c++11 link_pkgconfig
PKGCONFIG += wayland-client

system(wayland-scanner client-header ../../protocols/ukui-startup-v1.xml $$OUT_PWD/wayland-ukui-startup-v1-client-protocol.h)
system(wayland-scanner private-code ../../protocols/ukui-startup-v1.xml  $$OUT_PWD/wayland-ukui-startup-v1-protocol.c)

system(wayland-scanner client-header ../../protocols/ukui-shell-v1.xml $$OUT_PWD/wayland-ukui-shell-v1-client-protocol.h)
system(wayland-scanner private-code ../../protocols/ukui-shell-v1.xml  $$OUT_PWD/wayland-ukui-shell-v1-protocol.c)

INCLUDEPATH += $$OUT_PWD

SOURCES += \
    ukuiwindowhelper.cpp \
    ukuistartup.cpp \
    wayland-ukui-startup-v1-protocol.c \
    wayland-ukui-shell-v1-protocol.c \
    ukuidisplay.cpp \

INSTALL_HEADERS += \
    ukuiwindowhelper.h \
    ukuistartup.h \

PRIVATE_HEADERS += \
    ukuidisplay_p.h \
    wayland-ukui-startup-v1-client-protocol.h \
    wayland-ukui-shell-v1-client-protocol.h \

HEADERS += \
    $${INSTALL_HEADERS} \
    $${PRIVATE_HEADERS}

# Default rules for deployment.
headers.files = $${INSTALL_HEADERS}
headers.path = /usr/include/kysdk/applications/ukuiwindowhelper/
target.path = $$[QT_INSTALL_LIBS]
INSTALLS += target headers

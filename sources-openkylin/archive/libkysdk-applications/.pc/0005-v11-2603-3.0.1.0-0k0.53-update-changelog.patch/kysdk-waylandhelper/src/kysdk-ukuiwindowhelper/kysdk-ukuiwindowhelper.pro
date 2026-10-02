QT += gui widgets

TEMPLATE = lib

CONFIG += c++11 link_pkgconfig
PKGCONFIG += wayland-client

system(wayland-scanner client-header ../../protocols/ukui-startup-v1.xml $$OUT_PWD/wayland-ukui-startup-v1-client-protocol.h)
system(wayland-scanner private-code ../../protocols/ukui-startup-v1.xml  $$OUT_PWD/wayland-ukui-startup-v1-protocol.c)

system(wayland-scanner client-header ../../protocols/ukui-shell-v1.xml $$OUT_PWD/wayland-ukui-shell-v1-client-protocol.h)
system(wayland-scanner private-code ../../protocols/ukui-shell-v1.xml  $$OUT_PWD/wayland-ukui-shell-v1-protocol.c)

system(wayland-scanner client-header ../../protocols/kywc-capture-v1.xml $$OUT_PWD/wayland-kywc-capture-v1-client-protocol.h)
system(wayland-scanner private-code ../../protocols/kywc-capture-v1.xml  $$OUT_PWD/wayland-kywc-capture-v1-protocol.c)

system(wayland-scanner client-header ../../protocols/keystate.xml $$OUT_PWD/wayland-keystate-client-protocol.h)
system(wayland-scanner private-code ../../protocols/keystate.xml  $$OUT_PWD/wayland-keystate-protocol.c)

system(wayland-scanner client-header ../../protocols/virtual-keyboard-unstable-v1.xml $$OUT_PWD/wayland-virtual-keyboard-unstable-v1-client-protocol.h)
system(wayland-scanner private-code ../../protocols/virtual-keyboard-unstable-v1.xml  $$OUT_PWD/wayland-virtual-keyboard-unstable-v1-protocol.c)

INCLUDEPATH += $$OUT_PWD

SOURCES += \
    ukuiwindowhelper.cpp \
    ukuistartup.cpp \
    wayland-ukui-startup-v1-protocol.c \
    wayland-ukui-shell-v1-protocol.c \
    wayland-kywc-capture-v1-protocol.c \
    wayland-keystate-protocol.c \
    wayland-virtual-keyboard-unstable-v1-protocol.c \
    ukuidisplay.cpp \
    ukuiglobalcursor.cpp \
    ukuikeystate.cpp \
    ukuivirtualkeyboard.cpp \

INSTALL_HEADERS += \
    ukuiwindowhelper.h \
    ukuistartup.h \
    ukuiglobalcursor.h \
    ukuikeystate.h \
    ukuivirtualkeyboard.h \

PRIVATE_HEADERS += \
    ukuidisplay_p.h \
    wayland-ukui-startup-v1-client-protocol.h \
    wayland-ukui-shell-v1-client-protocol.h \
    wayland-kywc-capture-v1-client-protocol.h \
    wayland-virtual-keyboard-unstable-v1-client-protocol.h \

HEADERS += \
    $${INSTALL_HEADERS} \
    $${PRIVATE_HEADERS}

# Default rules for deployment.
headers.files = $${INSTALL_HEADERS}
headers.path = /usr/include/kysdk/applications/ukuiwindowhelper/
target.path = $$[QT_INSTALL_LIBS]
INSTALLS += target headers

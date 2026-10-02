INCLUDEPATH += $$PWD

include(menu-iface/menu-iface.pri)
include(directory-view-menu/directory-view-menu.pri)
include(side-bar-menu/side-bar-menu.pri)

HEADERS += \
    $$PWD/color-pushbutton.h \
    $$PWD/idm-actions-manager.h \
    $$PWD/menu-plugin-manager.h

SOURCES += \
    $$PWD/color-pushbutton.cpp \
    $$PWD/idm-actions-manager.cpp \
    $$PWD/menu-plugin-manager.cpp

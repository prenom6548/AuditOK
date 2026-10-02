QT += core gui widgets dbus
CONFIG        += c++11   link_pkgconfig

DEFINES += QT_DEPRECATED_WARNINGS

FORMS +=

HEADERS += \
    $$PWD/displayservice.h \
    $$PWD/displayswitch.h \
    $$PWD/greeterscreen.h \
    $$PWD/ui_mainwindow.h \
    $$PWD/xrandroutput.h

SOURCES += \
    $$PWD/displayservice.cpp \
    $$PWD/displayswitch.cpp \
    $$PWD/greeterscreen.cpp 

RESOURCES += \
    $$PWD/ds-res.qrc



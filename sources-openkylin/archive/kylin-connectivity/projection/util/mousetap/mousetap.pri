HEADERS += \
    $$PWD/mousetap.h

SOURCES += \
    $$PWD/mousetap.cpp

HEADERS += $$PWD/xmousetap.h
SOURCES += $$PWD/xmousetap.cpp
LIBS    += -lxcb
QT      += x11extras

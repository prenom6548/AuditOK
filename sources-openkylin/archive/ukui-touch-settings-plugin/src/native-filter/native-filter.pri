INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/devicemonitoreventfilter.h \
    $$PWD/geeventmemmover.h

SOURCES += \
    $$PWD/devicemonitoreventfilter.cpp

LIBS += -lxcb-xinput -lxcb

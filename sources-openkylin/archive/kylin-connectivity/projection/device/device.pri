include ($$PWD/android/android.pri)
include ($$PWD/controller/controller.pri)
include ($$PWD/decoder/decoder.pri)
include ($$PWD/deviceui/deviceui.pri)
#include ($$PWD/filehandler/filehandler.pri)
#include ($$PWD/recorder/recorder.pri)
include ($$PWD/render/render.pri)
include ($$PWD/server/server.pri)
include ($$PWD/stream/stream.pri)
include ($$PWD/usbdevice/usbdevice.pri)
include ($$PWD/wifidevice/wifidevice.pri)


HEADERS += \
    $$PWD/basedevice.h \
    $$PWD/baseserver.h

SOURCES += \
    $$PWD/basedevice.cpp \
    $$PWD/baseserver.cpp

INCLUDEPATH += \
        $$PWD/android \
        $$PWD/controller \
        $$PWD/decoder\
        $$PWD/deviceui \
#        $$PWD/filehandler \
#        $$PWD/recorder \
        $$PWD/render \
        $$PWD/server \
        $$PWD/stream \
        $$PWD/usbdevice \
        $$PWD/wifidevice


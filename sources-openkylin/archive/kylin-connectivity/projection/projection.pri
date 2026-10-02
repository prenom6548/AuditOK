include ($$PWD/device/device.pri)
include ($$PWD/devicecommon/devicecommon.pri)
include ($$PWD/devicemanage/devicemanage.pri)
include ($$PWD/fontawesome/fontawesome.pri)
include ($$PWD/uibase/uibase.pri)
include ($$PWD/util/util.pri)

QT += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

DEFINES += QT_DEPRECATED_WARNINGS

LIBS += -lavformat \
        -lavcodec \
        -lavutil \
        -lswscale

INCLUDEPATH += \
        $$PWD/device \
        $$PWD/devicecommon \
        $$PWD/devicemanage \
        $$PWD/fontawesome \
        $$PWD/uibase \
        $$PWD/util





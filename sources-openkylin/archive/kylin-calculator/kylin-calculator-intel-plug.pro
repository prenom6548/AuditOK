include (./kylin-calculator.pri)

TEMPLATE = lib
CONFIG += plugin

HEADERS += \
    $$PWD/intel_plug/calculator.h     \
    $$PWD/intel_plug/plug_interface.h

SOURCES += \
    $$PWD/intel_plug/calculator.cpp \


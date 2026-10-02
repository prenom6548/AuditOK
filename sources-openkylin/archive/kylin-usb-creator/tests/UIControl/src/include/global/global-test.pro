TEMPLATE = app
TARGET = test_global

QT += core widgets testlib

SOURCES += \
    test_global.cpp \
    ../../../../../UIControl/src/include/global.cpp

HEADERS += \
    test_global.h

INCLUDEPATH += \
    . \
    ../../../../../UIControl/src/include/ \
    /usr/include/KF5/KWindowSystem/ \
    /usr/include/x86_64-linux-gnu/qt5/QGSettings/

LIBS += -lgcov

QMAKE_CXXFLAGS += --coverage
QMAKE_LFLAGS += --coverage
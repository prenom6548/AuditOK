TEMPLATE = app
TARGET = test_menufactory

QT += core widgets testlib

SOURCES += \
    test_menufactory.cpp \
    main.cpp \
    ../../../../../UIControl/src/include/menufactory.cpp

HEADERS += \
    test_menufactory.h \
    ../../../../../UIControl/src/include/menufactory.h

INCLUDEPATH += \
    . \
    ../../../../../UIControl/src/include

# Coverage flags
QMAKE_CXXFLAGS += --coverage
QMAKE_LFLAGS += --coverage -lgcov
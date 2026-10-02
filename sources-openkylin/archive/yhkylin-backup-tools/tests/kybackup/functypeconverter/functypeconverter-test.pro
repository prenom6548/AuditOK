TEMPLATE = app
TARGET = test_functypeconverter

QT += testlib widgets

SOURCES += \
    main.cpp \
    test_functypeconverter.cpp \
    ../../../kybackup/functypeconverter.cpp

HEADERS += \
    test_functypeconverter.h \
    ../../../kybackup/functypeconverter.h

INCLUDEPATH += ../../../kybackup

# 添加覆盖率标志
QMAKE_CXXFLAGS += --coverage
QMAKE_LFLAGS += --coverage -lgcov
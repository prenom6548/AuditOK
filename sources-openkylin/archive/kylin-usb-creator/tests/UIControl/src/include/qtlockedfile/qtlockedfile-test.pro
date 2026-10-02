QT += core gui widgets testlib
CONFIG += c++11 console
CONFIG -= app_bundle

TEMPLATE = app
TARGET = test_qtlockedfile

INCLUDEPATH += ../../../.. ../../../../UIControl/src/include

SOURCES += \
    test_qtlockedfile.cpp \
    main.cpp \
    ../../../../../UIControl/src/include/qtlockedfile.cpp \
    ../../../../../UIControl/src/include/qtlockedfile_unix.cpp

HEADERS += \
    test_qtlockedfile.h

# 添加覆盖率标志
QMAKE_CXXFLAGS += --coverage -fno-exceptions
QMAKE_LFLAGS += --coverage -lgcov
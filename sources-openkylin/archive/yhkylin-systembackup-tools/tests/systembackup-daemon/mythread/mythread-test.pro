QT += core testlib
QT -= gui

TARGET = test_mythread
CONFIG += console
CONFIG -= app_bundle
CONFIG += c++11

QMAKE_CXXFLAGS +=  -fno-exceptions

# Coverage flags
QMAKE_CXXFLAGS += --coverage
QMAKE_LFLAGS += --coverage -lgcov

# Add QtTest module
QT += testlib

# Include paths
INCLUDEPATH += ../../common
INCLUDEPATH += ../../../systembackup-daemon

# Source files
HEADERS += \
    test_mythread.h \
    ../../../systembackup-daemon/mythread.h

SOURCES += \
    test_mythread.cpp \
    main.cpp \
    ../../../systembackup-daemon/mythread.cpp

# Libraries
LIBS += -lpthread

TEMPLATE = app
TARGET = test_protocol_analysis
CONFIG += console gtest gmock
CONFIG -= app_bundle

# Include paths
INCLUDEPATH += /usr/include
INCLUDEPATH += ../../../..
INCLUDEPATH += ../../../../src
INCLUDEPATH += ../../../../src/network

# Source files
SOURCES += \
    main.cpp \
    test_protocol_analysis.cpp \
    ../../../../src/network/protocol_analysis.cpp

# Header files
HEADERS += \
    test_protocol_analysis.h \
    ../../../../src/network/protocol_analysis.h

# Qt modules
QT += core

# Coverage flags
DEFINES += TEST_MODE
QMAKE_CXXFLAGS += --coverage -fbranch-count-reg -fno-exceptions
QMAKE_LDFLAGS += --coverage
LIBS += -lgcov -lgtest -lgmock -lpthread


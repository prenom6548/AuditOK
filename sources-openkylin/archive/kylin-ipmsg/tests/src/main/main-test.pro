QT += core testlib
TEMPLATE = app
TARGET = test_main

# Remove GUI dependencies for simpler testing
CONFIG += console gtest gmock
QT += core testlib

DEFINES += TEST_MODE
QMAKE_CXXFLAGS += --coverage -fbranch-count-reg
QMAKE_LDFLAGS += --coverage
LIBS += -lgcov

# Source files for the test
SOURCES += \
    main.cpp \
    test_main.cpp

# Header files
HEADERS += \
    test_main.h

# Include paths
INCLUDEPATH += ../../../src

# Libraries
LIBS += -lgtest -lgmock -lpthread

target.path = /tmp/tests
INSTALLS += target

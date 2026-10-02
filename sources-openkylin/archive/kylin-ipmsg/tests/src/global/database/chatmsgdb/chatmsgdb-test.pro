TEMPLATE = app
TARGET = test_chatmsgdb

# Include paths
INCLUDEPATH += /usr/include
INCLUDEPATH += ../../../..
INCLUDEPATH += ../../../../../src
INCLUDEPATH += ../../../../../src/global
INCLUDEPATH += ../../../../../src/global/database

# Source files
SOURCES += \
    main.cpp \
    test_chatmsgdb.cpp \
    ../../../../../src/global/database/chatmsgdb.cpp \
    ../../../../../src/global/database/database.cpp

# Header files
HEADERS += \
    test_chatmsgdb.h \
    ../../../../../src/global/database/chatmsgdb.h \
    ../../../../../src/global/database/database.h

# Qt modules
QT += sql testlib widgets

# Google Test and Google Mock
CONFIG += gtest gmock

# Link Google Test libraries
LIBS += -lgtest -lgmock

# Add coverage flags
DEFINES += TEST_MODE KYLIN_IPMSG_TEST
QMAKE_CXXFLAGS += --coverage -fbranch-count-reg -fno-exceptions
QMAKE_LDFLAGS += --coverage
LIBS += -lgcov

TEMPLATE = app
TARGET = test_globalutils
CONFIG += console gtest gmock
CONFIG -= app_bundle

# Include paths
INCLUDEPATH += ../../../..
INCLUDEPATH += ../../../../../src
INCLUDEPATH += ../../../../../src/global/utils

# Source files
SOURCES += \
    main.cpp \
    test_globalutils.cpp \
    ../../../../../src/global/utils/globalutils.cpp \
    ../../../../../src/global/utils/inisetting.cpp

# Header files
HEADERS += \
    ../../../../../src/global/utils/globalutils.h \
    ../../../../../src/global/utils/inisetting.h

# Qt modules
QT += core network

# 添加覆盖率标志
DEFINES += TEST_MODE
QMAKE_CXXFLAGS += --coverage -fbranch-count-reg -fno-exceptions
QMAKE_LDFLAGS += --coverage
LIBS += -lgcov -lgtest -lgmock -lpthread


QT += core testlib network

TARGET = test_inisetting
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app

# Source files
SOURCES += \
    main.cpp \
    test_inisetting.cpp \
    ../../../../../src/global/utils/inisetting.cpp

# Header files
HEADERS += \
    test_inisetting.h \
    ../../../../../src/global/utils/inisetting.h

# Include paths
INCLUDEPATH += \
    ../../../../../src \
    ../../../../../src/global \
    ../../../../../src/global/utils

# 添加覆盖率标志
DEFINES += TEST_MODE QT_DEPRECATED_WARNINGS
QMAKE_CXXFLAGS += --coverage -fbranch-count-reg -fno-exceptions
QMAKE_LDFLAGS += --coverage
LIBS += -lgcov

# Google Test linking
LIBS += -lgtest -lgtest_main

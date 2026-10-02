QT += core testlib network widgets

TARGET = test_addrset
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app

# Source files
SOURCES += \
    main.cpp \
    test_addrset.cpp \
    ../../../../../src/global/utils/addrset.cpp

# Header files
HEADERS += \
    test_addrset.h \
    ../../../../../src/global/utils/addrset.h

# Include paths
INCLUDEPATH += \
    ../../../../../src \
    ../../../../../src/global \
    ../../../../../src/global/utils \
    ../../../../../src/view/localinfo

# 添加覆盖率标志
DEFINES += TEST_MODE QT_DEPRECATED_WARNINGS
QMAKE_CXXFLAGS += --coverage -fbranch-count-reg -fno-exceptions
QMAKE_LDFLAGS += --coverage
LIBS += -lgcov -lgtest -lgtest_main

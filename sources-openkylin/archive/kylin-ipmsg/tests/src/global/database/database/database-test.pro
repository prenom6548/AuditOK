QT += core testlib sql
TEMPLATE = app
TARGET = test_database

CONFIG += gtest gmock console
INCLUDEPATH += /usr/include

DEFINES += TEST_MODE
QMAKE_CXXFLAGS += --coverage -fbranch-count-reg -fno-exceptions
QMAKE_LDFLAGS += --coverage
LIBS += -lgcov

# Source files for the test
SOURCES += \
    main.cpp \
    test_database.cpp

# Header files
HEADERS += \
    test_database.h

# Libraries
LIBS += -lgtest -lgmock -lpthread

target.path = /tmp/tests
INSTALLS += target

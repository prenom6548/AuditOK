QT += core testlib sql widgets
TEMPLATE = app
TARGET = test_friendlistdb

CONFIG += link_pkgconfig

DEFINES += TEST_MODE
QMAKE_CXXFLAGS += --coverage -fbranch-count-reg -fno-exceptions
QMAKE_LDFLAGS += --coverage
LIBS += -lgcov

# Source files for the test
SOURCES += \
    main.cpp \
    test_friendlistdb.cpp \
    ../../../../../src/global/database/friendlistdb.cpp \
    ../../../../../src/global/database/database.cpp

# Header files
HEADERS += \
    test_friendlistdb.h \
    ../../../../../src/global/database/friendlistdb.h \
    ../../../../../src/global/database/database.h

# Include paths
INCLUDEPATH += /usr/include
INCLUDEPATH += ../../../../../src/global/database \
               ../../../../../src/global \
               ../../../../../src

# Libraries
LIBS += -lgtest -lgmock -lpthread

target.path = /tmp/tests
INSTALLS += target

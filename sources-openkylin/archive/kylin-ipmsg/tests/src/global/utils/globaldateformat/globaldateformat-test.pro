QT += core dbus network
TEMPLATE = app
TARGET = test_globaldateformat

CONFIG += console gtest gmock link_pkgconfig

DEFINES += TEST_MODE
QMAKE_CXXFLAGS += --coverage -fbranch-count-reg -fno-exceptions
QMAKE_LDFLAGS += --coverage
LIBS += -lgcov

PKGCONFIG += kysdk-systime

SOURCES += \
    main.cpp \
    test_globaldateformat.cpp \
    ../../../../../src/global/utils/globaldateformat.cpp

HEADERS += \
    test_globaldateformat.h \
    ../../../../../src/global/utils/globaldateformat.h

INCLUDEPATH += ../../../../../src \
               ../../../../../src/global/utils

LIBS += -lgtest -lgmock -lpthread

target.path = /tmp/tests
INSTALLS += target

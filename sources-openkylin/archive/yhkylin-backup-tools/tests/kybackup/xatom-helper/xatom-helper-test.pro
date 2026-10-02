TEMPLATE = app
TARGET = test_xatom-helper

# Qt modules
QT += core gui widgets testlib x11extras

# C++ standard
CONFIG += c++14

# Include paths
INCLUDEPATH += $$PWD/../../../kybackup
INCLUDEPATH += $$PWD/../../../common

# Source files
SOURCES += \
    test_xatom-helper.cpp \
    main.cpp \
    ../../../kybackup/xatom-helper.cpp

# Header files
HEADERS += \
    test_xatom-helper.h \
    ../../../kybackup/xatom-helper.h

# Coverage flags
QMAKE_CXXFLAGS += --coverage
QMAKE_LFLAGS += --coverage -lgcov

# Define any necessary macros
DEFINES += UNIT_TEST QT_DEPRECATED_WARNINGS

# Link libraries
LIBS += -lpthread
LIBS += -lX11
LIBS += -lgtest -lgtest_main
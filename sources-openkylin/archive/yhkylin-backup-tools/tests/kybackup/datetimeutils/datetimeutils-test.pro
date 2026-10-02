TEMPLATE = app
TARGET = test_datetimeutils

# Qt modules
QT += core gui widgets testlib dbus

# C++ standard
CONFIG += c++14

# 配置gsettings
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt
PKGCONFIG += kysdk-systime

# Include paths
INCLUDEPATH += $$PWD/../../../kybackup
INCLUDEPATH += $$PWD/../../../common

# Source files
SOURCES += \
    test_datetimeutils.cpp \
    main.cpp \
    ../../../kybackup/datetimeutils.cpp

# Header files
HEADERS += \
    test_datetimeutils.h \
    ../../../kybackup/datetimeutils.h

# Coverage flags
QMAKE_CXXFLAGS += --coverage
QMAKE_LFLAGS += --coverage -lgcov

# Define any necessary macros
DEFINES += UNIT_TEST

# Link libraries
LIBS += -lpthread
LIBS += -lgtest -lgtest_main
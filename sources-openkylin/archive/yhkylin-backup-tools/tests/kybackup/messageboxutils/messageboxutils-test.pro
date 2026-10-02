TEMPLATE = app
TARGET = test_messageboxutils

# Qt modules
QT += core gui widgets testlib dbus xml

# C++ standard
CONFIG += c++14

# Include paths
INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/../../..
INCLUDEPATH += $$PWD/../../../common
INCLUDEPATH += $$PWD/../../../kybackup
INCLUDEPATH += /usr/include/kysdk/applications

# 配置gsettings
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt

# Source files
SOURCES += \
    test_messageboxutils.cpp \
    main.cpp \
    ../../../kybackup/messageboxutils.cpp \
    ../../../kybackup/gsettingswrapper.cpp \
    ../../../common/mydefine.cpp

# Header files
HEADERS += \
    test_messageboxutils.h \
    ../../../kybackup/messageboxutils.h \
    ../../../kybackup/gsettingswrapper.h \
    ../../../kybackup/globalbackupinfo.h \
    ../../../kybackup/globalsignals.h \
    ../../../common/mydefine.h \
    ../../../common/singleton.h

# Coverage flags
QMAKE_CXXFLAGS += --coverage
QMAKE_LFLAGS += --coverage -lgcov

# Define any necessary macros
DEFINES += UNIT_TEST QT_DEPRECATED_WARNINGS

# Link libraries
LIBS += -lpthread
LIBS += -lgtest -lgtest_main
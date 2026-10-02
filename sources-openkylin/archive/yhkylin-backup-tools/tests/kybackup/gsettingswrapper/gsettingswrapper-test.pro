TEMPLATE = app
TARGET = test_gsettingswrapper

# Qt modules
QT += core gui widgets testlib dbus

# C++ standard
CONFIG += c++14

# 配置gsettings
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt

# Include paths
INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/../../..
INCLUDEPATH += $$PWD/../../../common
INCLUDEPATH += $$PWD/../../../kybackup

# Source files
SOURCES += \
    test_gsettingswrapper.cpp \
    main.cpp \
    ../../../kybackup/gsettingswrapper.cpp \
    mock_globalsignals.cpp

# Header files
HEADERS += \
    test_gsettingswrapper.h \
    ../../../kybackup/gsettingswrapper.h \
    ../../../kybackup/globalsignals.h \
    ../../../kybackup/globalbackupinfo.h \
    mock_globalsignals.h

# Coverage flags
QMAKE_CXXFLAGS += --coverage
QMAKE_LFLAGS += --coverage -lgcov

# Define any necessary macros
DEFINES += UNIT_TEST

# Link libraries
LIBS += -lpthread
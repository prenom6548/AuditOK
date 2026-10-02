#-------------------------------------------------
#
# Project created by QtCreator 2019-08-22T11:12:59
#
#-------------------------------------------------
QT += core
QT -= gui
TEMPLATE = app

TARGET = afltest_log4qt
target.source += $$TARGET
target.path = ./

#AFL++测试工具
QMAKE_CC = afl-cc
QMAKE_CXX = afl-c++
QMAKE_LINK = afl-c++
QMAKE_LFLAGS += -fsanitize=fuzzer
QMAKE_CXXFLAGS += -fsanitize=leak,fuzzer
QMAKE_CFLAGS += -fsanitize=leak,fuzzer

LIBS          +=
LIBS          +=  -L$$[QT_INSTALL_LIBS]

CONFIG += link_pkgconfig
PKGCONFIG += 

#log4qt
include($$PWD/../../src/log4qt/ukui-log4qt.pri)

# Include directories
INCLUDEPATH += $$PROJECT_ROOTDIR \
               $$PWD/../../src/log4qt

SOURCES += fuzz_target.cpp

HEADERS += 


#-------------------------------------------------
#
# Project created by QtCreator 2019-08-22T11:12:59
#
#-------------------------------------------------
QT += core
QT -= gui
TEMPLATE = app

TARGET = afltest_common
target.source += $$TARGET
target.path = ./

#AFL++测试工具
QMAKE_CC = afl-cc
QMAKE_CXX = afl-c++
QMAKE_LINK = afl-c++
QMAKE_LFLAGS += -fsanitize=fuzzer
QMAKE_CXXFLAGS += -fsanitize=leak,fuzzer
QMAKE_CFLAGS += -fsanitize=leak,fuzzer

LIBS          += -lpthread -liniparser
LIBS          +=  -L$$[QT_INSTALL_LIBS]

CONFIG += link_pkgconfig
PKGCONFIG += 

# Include directories
INCLUDEPATH += $$PROJECT_ROOTDIR \
               $$PWD/../../src/common

SOURCES += ../../src/common/kylin-com4cxx.cpp \
           ../../src/common/kylin-com4c.c \
           ../../src/common/kylin-ini.c \
           fuzz_target.cpp

HEADERS += ../../src/common/kylin-ini.h \
           ../../src/common/kylin-com4c.h \
           ../../src/common/kylin-com4cxx.h


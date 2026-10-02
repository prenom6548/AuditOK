#-------------------------------------------------
#
# Project created by QtCreator 2019-08-22T11:12:59
#
#-------------------------------------------------
QT += core
QT -= gui
TEMPLATE = app

TARGET = unit_test_common
target.source += $$TARGET
target.path = ./

#代码覆盖率工具gcov
QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage
QMAKE_CXXFLAGS += --coverage -fno-inline -fno-access-control -fno-exceptions

LIBS          += -lgtest_main -lpthread -liniparser
LIBS          +=  -L$$[QT_INSTALL_LIBS] -lgtest -lgcov

#OBJECTS_DIR = ./obj
#MOC_DIR = ./moc

CONFIG += link_pkgconfig
PKGCONFIG += 

#打桩工具
TEST_UTILS_PATH = $$PWD/../kt-test-utils

# Include directories
INCLUDEPATH += $$PROJECT_ROOTDIR \
               $$TEST_UTILS_PATH/cpp-stub \
               $$TEST_UTILS_PATH/cpp-stub-ext \
               $$PWD/../../src/common

SOURCES += unit_test_common.cpp \
           ../../src/common/kylin-com4cxx.cpp \
           ../../src/common/kylin-com4c.c \
           ../../src/common/kylin-ini.c \
           main.cpp

HEADERS += ../../src/common/kylin-ini.h \
           ../../src/common/kylin-com4c.h \
           ../../src/common/kylin-com4cxx.h


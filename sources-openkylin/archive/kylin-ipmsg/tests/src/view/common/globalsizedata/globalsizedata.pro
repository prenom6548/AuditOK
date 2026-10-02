TEMPLATE = app
TARGET = test_globalsizedata

QT += core gui widgets testlib

CONFIG += console
CONFIG -= app_bundle

SOURCES += \
    test_globalsizedata.cpp \
    main.cpp \
    ../../../../../src/view/common/globalsizedata.cpp

HEADERS += \
    test_globalsizedata.h

# 添加包含路径
INCLUDEPATH += \
    $$PWD/../../../../../ \
    ../../../../../src/view/common \
    ../../../../../src/view \
    ../../../../../src \
    ../../../../../ \
    ../../../../../kabase \
    /usr/include/kysdk/kysdk-base \
    /usr/include/x86_64-linux-gnu/qt5 \
    /usr/include/x86_64-linux-gnu/qt5/QtTest

# 添加源文件搜索路径
VPATH += ../../../../../src/view/common

# 添加覆盖率标志
DEFINES += TEST_MODE
QMAKE_CXXFLAGS += --coverage -fbranch-count-reg -fno-exceptions
QMAKE_LFLAGS += --coverage -lgcov

# 添加SDK库依赖
LIBS += $$system(pkg-config --libs kysdk-base) $$system(pkg-config --libs kysdk-diagnostics)

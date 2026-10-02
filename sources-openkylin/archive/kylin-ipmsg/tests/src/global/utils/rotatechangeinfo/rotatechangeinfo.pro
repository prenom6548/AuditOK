TEMPLATE = app
TARGET = test_rotatechangeinfo

QT += core gui widgets testlib dbus
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += link_pkgconfig
CONFIG -= app_bundle

SOURCES += \
    test_rotatechangeinfo.cpp \
    main.cpp \
    ../../../../../../src/global/utils/rotatechangeinfo.cpp

HEADERS += \
    test_rotatechangeinfo.h \
    ../../../../../../src/global/utils/rotatechangeinfo.h

# 添加包含路径
INCLUDEPATH += \
    ../../../../../.. \
    ../../../../../../src \
    ../../../../../../src/global \
    ../../../../../../src/global/utils

# 添加覆盖率标志
DEFINES += TEST_MODE
QMAKE_CXXFLAGS += --coverage -fbranch-count-reg -fno-exceptions
QMAKE_LDFLAGS += --coverage
LIBS += -lgcov

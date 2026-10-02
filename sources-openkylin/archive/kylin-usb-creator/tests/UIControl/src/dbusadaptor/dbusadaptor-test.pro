QT += testlib core dbus widgets
QT -= gui

TARGET = test_dbusadaptor
TEMPLATE = app

CONFIG += c++11 console
CONFIG -= app_bundle

# 包含覆盖率标志
QMAKE_CXXFLAGS += --coverage -fno-exceptions
QMAKE_LFLAGS += --coverage -lgcov

SOURCES += \
    test_dbusadaptor.cpp \
    main.cpp

HEADERS += \
    test_dbusadaptor.h
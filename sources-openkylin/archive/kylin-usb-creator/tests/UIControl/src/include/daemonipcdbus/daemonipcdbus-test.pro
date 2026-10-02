QT += testlib core dbus widgets
QT -= gui

TARGET = test_daemonipcdbus
TEMPLATE = app

CONFIG += c++11 console
CONFIG -= app_bundle

# 包含覆盖率标志
QMAKE_CXXFLAGS += --coverage -fno-exceptions
QMAKE_LFLAGS += --coverage -lgcov

SOURCES += \
    test_daemonipcdbus.cpp \
    main.cpp \
    ../../../../../UIControl/src/include/daemonipcdbus.cpp

HEADERS += \
    test_daemonipcdbus.h \
    ../../../../../UIControl/src/include/daemonipcdbus.h
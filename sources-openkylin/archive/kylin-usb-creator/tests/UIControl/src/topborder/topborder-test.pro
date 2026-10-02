TEMPLATE = app
TARGET = test_topborder

QT += core gui widgets testlib

SOURCES += \
    test_topborder.cpp \
    ../../../../UIControl/src/topborder.cpp

HEADERS += \
    test_topborder.h \
    ../../../../UIControl/src/topborder.h

INCLUDEPATH += \
    ../../../../UIControl/src

# 覆盖率标志
QMAKE_CXXFLAGS += --coverage -fno-exceptions
QMAKE_LDFLAGS += --coverage
LIBS += -lgcov

# 链接必要的Qt库
LIBS += -lpthread -lX11
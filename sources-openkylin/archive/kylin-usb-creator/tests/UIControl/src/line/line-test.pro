TEMPLATE = app
TARGET = test_line

QT += core gui widgets testlib

SOURCES += \
    test_line.cpp \
    ../../../../UIControl/src/line.cpp

HEADERS += \
    test_line.h \
    ../../../../UIControl/src/line.h

INCLUDEPATH += \
    ../../../../UIControl/src

# 覆盖率标志
QMAKE_CXXFLAGS += --coverage -fno-exceptions
QMAKE_LDFLAGS += --coverage
LIBS += -lgcov

# 链接必要的Qt库
LIBS += -lpthread -lX11
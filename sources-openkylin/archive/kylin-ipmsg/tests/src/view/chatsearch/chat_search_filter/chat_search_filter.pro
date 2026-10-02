TEMPLATE = app
TARGET = test_chat_search_filter

QT += core gui widgets testlib
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += link_pkgconfig
CONFIG -= app_bundle

# 添加pkg-config包
PKGCONFIG += gsettings-qt

SOURCES += \
    test_chat_search_filter.cpp \
    main.cpp \
    ../../../../../../src/view/chatsearch/chat_search_filter.cpp

HEADERS += \
    test_chat_search_filter.h \
    ../../../../../../src/view/chatsearch/chat_search_filter.h

# 添加包含路径
INCLUDEPATH += \
    ../../../../../.. \
    ../../../../../../src \
    ../../../../../../src/view \
    ../../../../../../src/view/chatsearch \
    ../../../../../../src/model \
    ../../../../../../src/global \
    ../../../../../../src/global/utils \
    ../../../../../../src/global/database

# 添加覆盖率标志
DEFINES += TEST_MODE
QMAKE_CXXFLAGS += --coverage -fbranch-count-reg -fno-exceptions
QMAKE_LDFLAGS += --coverage

# 链接库
LIBS += -lgcov

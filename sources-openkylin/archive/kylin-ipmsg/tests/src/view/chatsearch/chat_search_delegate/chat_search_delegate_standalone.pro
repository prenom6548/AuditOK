TEMPLATE = app
TARGET = test_chat_search_delegate_standalone

QT += core gui widgets testlib
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += link_pkgconfig
CONFIG -= app_bundle

SOURCES += \
    test_chat_search_delegate_standalone.cpp

# 添加包含路径
INCLUDEPATH += \
    ../../../../../.. \
    ../../../../../../src \
    ../../../../../../src/view \
    ../../../../../../src/view/chatsearch

# kysdk相关包含路径
INCLUDEPATH += \
    /usr/include/kysdk/applications \
    /usr/include/kysdk/applications/ukuiwindowhelper \
    /usr/include/KF5 \
    /usr/include/KF5/KWindowSystem \
    /usr/include/kysdk/kysdk-system \
    /usr/include/kysdk/kysdk-base

# 添加覆盖率标志
QMAKE_CXXFLAGS += --coverage
QMAKE_LFLAGS += --coverage -lgcov

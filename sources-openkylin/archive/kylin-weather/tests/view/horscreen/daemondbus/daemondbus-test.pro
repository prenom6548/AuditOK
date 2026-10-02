QT += core gui testlib widgets dbus
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt
TEMPLATE = app

TARGET = test_daemondbus

# 添加覆盖率标志
QMAKE_CXXFLAGS += --coverage -fno-exceptions -fbranch-count-reg
QMAKE_LFLAGS += --coverage -lgcov

# 包含源文件
SOURCES += \
    test_daemondbus.cpp \
    main.cpp \
    ../../../../view/horscreen/daemondbus.cpp

# 包含头文件
HEADERS += \
    test_daemondbus.h \
    ../../../../view/horscreen/daemondbus.h

# 包含路径
INCLUDEPATH += ../../../../view/horscreen ../../../../global ../../../..

# 链接Qt模块
QT += widgets testlib dbus
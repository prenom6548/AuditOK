QT += core gui testlib widgets
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt
TEMPLATE = app

TARGET = test_empty

# 添加覆盖率标志
QMAKE_CXXFLAGS += --coverage -fno-exceptions -fbranch-count-reg
QMAKE_LFLAGS += --coverage -lgcov

# 包含源文件
SOURCES += \
    test_empty.cpp \
    main.cpp \
    ../../../../view/horscreen/empty.cpp

# 包含头文件
HEADERS += \
    test_empty.h \
    ../../../../view/horscreen/empty.h

# 包含路径
INCLUDEPATH += ../../../../view/horscreen ../../../../global ../../../..

# 链接Qt模块
QT += widgets testlib
QT += core gui testlib widgets
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt
TEMPLATE = app

TARGET = test_promptwidget

# 添加覆盖率标志
QMAKE_CXXFLAGS += --coverage -fno-exceptions -fbranch-count-reg
QMAKE_LFLAGS += --coverage -lgcov

# 链接kysdk-kabase库
LIBS += -lkysdk-kabase

# 包含源文件
SOURCES += \
    test_promptwidget.cpp \
    main.cpp \
    ../../../../view/horscreen/promptwidget.cpp \
    ../../../../global/viewvar.cpp \
    ../../../../global/rotatechangeinfo.cpp

# 包含头文件
HEADERS += \
    test_promptwidget.h \
    ../../../../view/horscreen/promptwidget.h \
    ../../../../global/viewvar.h \
    ../../../../global/rotatechangeinfo.h

# 包含路径
INCLUDEPATH += ../../../../view/horscreen ../../../../global ../../../.. /usr/include/kysdk/applications/kabase /usr/include/kysdk/applications/kabase/kylin_system

# 链接Qt模块
QT += widgets testlib dbus
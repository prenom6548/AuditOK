# CoreInteraction Test Project

QT += testlib widgets network core dbus x11extras
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += link_pkgconfig c++11 testcase
TEMPLATE = app

TARGET = test_coreinteraction

# 添加源文件
SOURCES += \
    test_coreinteraction.cpp \
    main.cpp \
    ../../../../controller/core/coreinteraction.cpp \
    ../../../../controller/core/core.cpp \
    ../../../../controller/core/errorreact.cpp \
    ../../../../global/corevar.cpp \
    ../../../../global/viewvar.cpp \
    ../../../../global/rotatechangeinfo.cpp \
    ../../../../model/dataparser.cpp \
    ../../../../model/datarequest.cpp \
    ../../../../model/dbus.cpp \
    ../../../../model/iplocation.cpp \
    ../../../../model/network.cpp \
    ../../../../model/searchmarch.cpp \
    ../../../../model/titlesearchmodel.cpp

# 添加头文件
HEADERS += \
    test_coreinteraction.h \
    ../../../../controller/core/coreinteraction.h \
    ../../../../controller/core/core.h \
    ../../../../controller/core/errorreact.h \
    ../../../../controller/Interaction.h \
    ../../../../global/corevar.h \
    ../../../../global/viewvar.h \
    ../../../../global/rotatechangeinfo.h \
    ../../../../model/dataparser.h \
    ../../../../model/datarequest.h \
    ../../../../model/dbus.h \
    ../../../../model/iplocation.h \
    ../../../../model/network.h \
    ../../../../model/searchmarch.h \
    ../../../../model/titlesearchmodel.h

# 包含路径
INCLUDEPATH += \
    . \
    ../../../../ \
    ../../../../controller \
    ../../../../controller/core \
    ../../../../model \
    ../../../../global \
    ../../../../kabase \
    ../../../../kabase/Qt \
    /usr/include/kabase/ \
    /usr/include/kysdk/applications/kabase/kylin_system/ \
    /usr/include/kabase/kylin_system/ \
    /usr/include/kysdk/applications/kabase/

# pkgconfig
PKGCONFIG += kysdk-qtwidgets kysdk-log kysdk-waylandhelper kysdk-kabase gsettings-qt kysdk-datacollect kysdk-ukuiwindowhelper

# 库路径和链接
LIBS += -L/usr/lib/kysdk/kysdk-base -lkylog -lkyconf

# 覆盖率标志
QMAKE_CXXFLAGS += --coverage -fno-exceptions -fbranch-count-reg
QMAKE_LFLAGS += --coverage -lgcov

# 定义测试数据目录
DEFINES += SRCDIR=\\\"$$PWD/\\\"

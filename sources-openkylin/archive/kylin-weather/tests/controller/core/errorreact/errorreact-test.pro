QT += core gui testlib widgets
TEMPLATE = app

TARGET = test_errorreact

# 添加覆盖率标志
QMAKE_CXXFLAGS += --coverage -fno-exceptions -fbranch-count-reg
QMAKE_LFLAGS += --coverage -lgcov

# 包含源文件
SOURCES += \
    test_errorreact.cpp \
    main.cpp \
    ../../../../controller/core/errorreact.cpp

# 包含头文件
HEADERS += \
    test_errorreact.h \
    ../../../../controller/core/errorreact.h

# 包含路径
INCLUDEPATH += \
    . \
    ../../../ \
    ../../../controller \
    ../../../controller/core \
    ../../../model \
    ../../../global \
    ../../../view \
    ../../../view/horscreen \
    ../../../view/verscreen \
    ../../../kabase \
    ../../../kabase/Qt \
    /usr/include/kabase/ \
    /usr/include/kysdk/applications/kabase/kylin_system/ \
    /usr/include/kabase/kylin_system/ \
    /usr/include/kysdk/applications/kabase/ \
    /usr/include/x86_64-linux-gnu/qt5/QGSettings

# pkgconfig
PKGCONFIG += kysdk-qtwidgets kysdk-log kysdk-waylandhelper kysdk-kabase gsettings-qt kysdk-datacollect kysdk-ukuiwindowhelper

# 库路径和链接
LIBS += -L/usr/lib/kysdk/kysdk-base -lkylog -lkyconf

# 定义测试数据目录
DEFINES += SRCDIR=\"$$PWD/\"

QT += core gui testlib widgets dbus
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt kysdk-kabase kysdk-datacollect
TEMPLATE = app

TARGET = test_weekweather

# 添加覆盖率标志
QMAKE_CXXFLAGS += --coverage -fno-exceptions -fbranch-count-reg
QMAKE_LFLAGS += --coverage -lgcov

# 添加kysdk库和包含路径
LIBS += -lkylog -lkydiagnostics -lkyconf -lsystemd -lkydatastruct -lkyutils
INCLUDEPATH += /usr/include/kysdk/kysdk-base/ /usr/include/kysdk/applications/kabase/ /usr/include/kysdk/applications/kabase/kylin_system/ /usr/include/kysdk/applications/ /usr/include/kysdk/applications/ukuiwindowhelper/ /usr/include/KF5/KWindowSystem/ ../../../../kabase ../../../../kabase/Qt

# 包含源文件
SOURCES += \
    test_weekweather.cpp \
    ../../../../view/horscreen/weekweather.cpp \
    ../../../../view/horscreen/onedayweather.cpp \
    ../../../../view/horscreen/splitline.cpp \
    ../../../../global/viewvar.cpp \
    ../../../../global/rotatechangeinfo.cpp

# 包含头文件
HEADERS += \
    test_weekweather.h \
    ../../../../view/horscreen/weekweather.h \
    ../../../../view/horscreen/onedayweather.h \
    ../../../../view/horscreen/splitline.h \
    ../../../../global/viewvar.h \
    ../../../../global/rotatechangeinfo.h \
    ../../../../global/corevar.h

# 包含路径
INCLUDEPATH += ../../../../view/horscreen ../../../../global ../../../.. ../../..

# 链接Qt模块
QT += widgets testlib dbus
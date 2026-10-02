QT += core gui testlib widgets dbus
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt kysdk-kabase kysdk-datacollect
TEMPLATE = app

TARGET = test_information

# 添加覆盖率标志
QMAKE_CXXFLAGS += --coverage -fno-exceptions -fbranch-count-reg
QMAKE_LFLAGS += --coverage -lgcov

# 添加kysdk库和包含路径
LIBS += -lkylog -lkydiagnostics -lkyconf -lsystemd -lkydatastruct -lkyutils
INCLUDEPATH += /usr/include/kysdk/kysdk-base/ /usr/include/kysdk/applications/kabase/ /usr/include/kysdk/applications/kabase/kylin_system/ /usr/include/kysdk/applications/ /usr/include/kysdk/applications/ukuiwindowhelper/ /usr/include/KF5/KWindowSystem/ ../../../../kabase ../../../../kabase/Qt

# 包含源文件
SOURCES += \
    test_information.cpp \
    ../../../../view/horscreen/information.cpp \
    ../../../../view/horscreen/perhour.cpp \
    ../../../../view/horscreen/weekweather.cpp \
    ../../../../view/horscreen/lifestyle.cpp \
    ../../../../view/horscreen/onehourweather.cpp \
    ../../../../view/horscreen/onedayweather.cpp \
    ../../../../view/horscreen/splitline.cpp \
    ../../../../view/horscreen/ultravioletrays.cpp \
    ../../../../view/horscreen/air.cpp \
    ../../../../view/horscreen/sport.cpp \
    ../../../../view/horscreen/washcar.cpp \
    ../../../../view/horscreen/flu.cpp \
    ../../../../view/horscreen/clothe.cpp \
    ../../../../global/corevar.cpp \
    ../../../../global/viewvar.cpp \
    ../../../../global/rotatechangeinfo.cpp

# 包含头文件
HEADERS += \
    test_information.h \
    ../../../../view/horscreen/information.h \
    ../../../../view/horscreen/perhour.h \
    ../../../../view/horscreen/weekweather.h \
    ../../../../view/horscreen/lifestyle.h \
    ../../../../view/horscreen/onehourweather.h \
    ../../../../view/horscreen/onedayweather.h \
    ../../../../view/horscreen/splitline.h \
    ../../../../view/horscreen/ultravioletrays.h \
    ../../../../view/horscreen/air.h \
    ../../../../view/horscreen/sport.h \
    ../../../../view/horscreen/washcar.h \
    ../../../../view/horscreen/flu.h \
    ../../../../view/horscreen/clothe.h \
    ../../../../global/corevar.h \
    ../../../../global/viewvar.h \
    ../../../../global/rotatechangeinfo.h

# 包含路径
INCLUDEPATH += ../../../../view/horscreen ../../../../global ../../../.. ../../..

# 链接Qt模块
QT += widgets testlib dbus
QT += core gui testlib widgets dbus
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt kysdk-qtwidgets kysdk-log kysdk-kabase
TEMPLATE = app

TARGET = test_lifestyle

# 添加覆盖率标志
QMAKE_CXXFLAGS += --coverage -fno-exceptions -fbranch-count-reg
QMAKE_LFLAGS += --coverage -lgcov

# 添加kysdk库和包含路径
LIBS += -lkylog -lkyconf
INCLUDEPATH += /usr/include/kysdk/kysdk-base/ /usr/include/kysdk/applications/kabase/ /usr/include/kysdk/applications/kabase/kylin_system/ /usr/include/kysdk/applications/ /usr/include/kabase/ ../../../../kabase ../../../../kabase/Qt

# 包含源文件
SOURCES += \
    test_lifestyle.cpp \
    main.cpp \
    ../../../../view/horscreen/lifestyle.cpp \
    ../../../../view/horscreen/ultravioletrays.cpp \
    ../../../../view/horscreen/air.cpp \
    ../../../../view/horscreen/sport.cpp \
    ../../../../view/horscreen/washcar.cpp \
    ../../../../view/horscreen/flu.cpp \
    ../../../../view/horscreen/clothe.cpp \
    ../../../../global/rotatechangeinfo.cpp \
    ../../../../global/corevar.cpp \
    ../../../../global/viewvar.cpp

# 包含头文件
HEADERS += \
    test_lifestyle.h \
    ../../../../view/horscreen/lifestyle.h \
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
INCLUDEPATH += ../../../../view/horscreen ../../../../global ../../../.. .

# 链接Qt模块
QT += widgets testlib dbus
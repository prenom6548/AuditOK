QT += core gui testlib widgets dbus
CONFIG += link_pkgconfig c++11
PKGCONFIG += gsettings-qt kysdk-kabase kysdk-qtwidgets
TEMPLATE = app

TARGET = test_city

# 添加覆盖率标志
QMAKE_CXXFLAGS += --coverage -fno-exceptions -fbranch-count-reg
QMAKE_LFLAGS += --coverage -lgcov

# 包含源文件
SOURCES += \
    test_city.cpp \
    main.cpp \
    ../../../../view/horscreen/city.cpp \
    ../../../../global/rotatechangeinfo.cpp

# 包含头文件
HEADERS += \
    test_city.h \
    ../../../../view/horscreen/city.h \
    ../../../../global/rotatechangeinfo.h

# 包含路径
INCLUDEPATH += . \
    ../../../../view/horscreen \
    ../../../../view/search \
    ../../../../global \
    ../../../../controller \
    ../../../../model \
    ../../../../ \
    /usr/include/glib-2.0 \
    /usr/lib/x86_64-linux-gnu/glib-2.0/include \
    /usr/include/libmount \
    /usr/include/blkid \
    /usr/include/x86_64-linux-gnu/qt5/QGSettings \
    /usr/include/kysdk/applications/kabase \
    /usr/include/kysdk/applications/kabase/kylin_system \
    /usr/include/kysdk/applications

# 链接Qt模块
QT += widgets testlib

# 链接项目所需的其他源文件
SOURCES += \
    ../../../../global/corevar.cpp \
    ../../../../global/viewvar.cpp

# 包含这些源文件的头文件
HEADERS += \
    ../../../../global/corevar.h \
    ../../../../global/viewvar.h

# 链接库路径
LIBS += -L/usr/lib/kysdk/kysdk-base -lkylog -lkyconf
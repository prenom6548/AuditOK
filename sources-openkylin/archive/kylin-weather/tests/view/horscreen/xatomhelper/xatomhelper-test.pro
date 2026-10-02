QT += core gui testlib widgets x11extras

CONFIG += link_pkgconfig
TEMPLATE = app

TARGET = test_xatomhelper

# 添加覆盖率标志
QMAKE_CXXFLAGS += --coverage -fno-exceptions -fbranch-count-reg
QMAKE_LFLAGS += --coverage -lgcov

# 添加包含路径
INCLUDEPATH += ../../../.. ../../..
INCLUDEPATH += ../../../../view/horscreen
INCLUDEPATH += /usr/include/KF5
INCLUDEPATH += /usr/include/KF5/KWindowSystem

# 包含源文件
SOURCES += \
    main.cpp \
    test_xatomhelper.cpp \
    ../../../../view/horscreen/xatom-helper.cpp

# 包含头文件
HEADERS += \
    test_xatomhelper.h \
    ../../../../view/horscreen/xatom-helper.h

# 添加X11库和KF5库
LIBS += -lX11 -lKF5WindowSystem

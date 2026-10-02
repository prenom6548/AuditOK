QT += core gui testlib widgets network dbus
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt kysdk-qtwidgets
TEMPLATE = app

TARGET = test_changecitywidget

# 添加覆盖率标志
QMAKE_CXXFLAGS += --coverage -fno-exceptions -fbranch-count-reg
QMAKE_LFLAGS += --coverage -lgcov

# 包含源文件
SOURCES += \
    test_changecitywidget.cpp \
    main.cpp \
    ../../../../view/horscreen/changecitywidget.cpp \
    ../../../../global/corevar.cpp \
    mock_interaction.cpp 

# 包含头文件
HEADERS += \
    test_changecitywidget.h \
    ../../../../view/horscreen/changecitywidget.h \
    ../../../../controller/Interaction.h \
    ../../../../global/corevar.h \
    mock_interaction.h 

# 包含路径
INCLUDEPATH += ../../../../view/horscreen ../../../../controller ../../../../global ../../../../controller/core ../../../..

# 链接Qt模块
QT += widgets testlib network dbus

# 链接kysdk库
LIBS += -lkylog -lkydiagnostics -lkyconf -lsystemd -lkydatastruct -lkyutils

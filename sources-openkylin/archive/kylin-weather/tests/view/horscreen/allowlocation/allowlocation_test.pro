QT += core gui testlib widgets
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt
TEMPLATE = app
TARGET = test_allowlocation

# 添加覆盖率标志
QMAKE_CXXFLAGS += --coverage
QMAKE_LFLAGS += --coverage -lgcov

# 包含源文件
SOURCES += \
    test_allowlocation.cpp \
    main.cpp \
    ../../../../view/horscreen/allowlocation.cpp \
    mock_symbols.cpp

# 包含头文件
HEADERS += \
    test_allowlocation.h \
    ../../../../view/horscreen/allowlocation.h \
    ../../../../controller/Interaction.h \
    ../../../../global/corevar.h

# 包含路径
INCLUDEPATH += . ../../../../view/horscreen ../../../../controller ../../../../global ../../../../kabase ../../../../kabase/Qt ../../../..

# 链接Qt模块
QT += widgets testlib

# 移除了kysdk库的链接，因为我们提供了模拟实现
# LIBS += -lkylog -lkydiagnostics -lkyconf -lsystemd -lkydatastruct -lkyutils

# 移除了kysdk包含路径
# INCLUDEPATH += /usr/include/kysdk/kysdk-base/ /usr/include/kysdk/applications/kabase/ /usr/include/kysdk/applications/kabase/kylin_system/ /usr/include/kysdk/applications /usr/include/kysdk/applications/ukuiwindowhelper/ /usr/include/KF5/KWindowSystem/

# 添加当前目录到包含路径，以便找到模拟头文件
INCLUDEPATH += .
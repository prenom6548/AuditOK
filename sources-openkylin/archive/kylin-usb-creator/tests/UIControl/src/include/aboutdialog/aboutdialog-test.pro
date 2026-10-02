QT += testlib widgets gui core dbus
TEMPLATE = app
TARGET = test_aboutdialog

# 包含被测源代码目录
INCLUDEPATH += ../../../../../UIControl/src

# 包含被测源代码文件
SOURCES += \
    test_aboutdialog.cpp \
    main.cpp \
    ../../../../../UIControl/src/include/aboutdialog.cpp \
    ../../../../../UIControl/src/include/global.cpp \
    ../../../../../UIControl/src/include/daemonipcdbus.cpp

HEADERS += \
    test_aboutdialog.h \
    ../../../../../UIControl/src/include/aboutdialog.h \
    ../../../../../UIControl/src/include/global.h \
    ../../../../../UIControl/src/include/daemonipcdbus.h \
    ../../../../../UIControl/src/include/constant_class.h

# 添加覆盖率标志
QMAKE_CXXFLAGS += --coverage -fno-exceptions
QMAKE_LFLAGS += --coverage -lgcov

# 链接所需库
LIBS += -lpthread -lX11 -lukui-log4qt

# 添加Qt模块和pkgconfig依赖
QT += widgets gui core dbus testlib x11extras KWindowSystem
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt kysdk-waylandhelper kysdk-qtwidgets kysdk-soundeffects kysdk-ukuiwindowhelper kysdk-datacollect

# 设置包含路径
INCLUDEPATH += ../../../../../UIControl/src/include

# 设置资源文件（暂时注释掉，因为我们不需要测试资源文件）
# RESOURCES += ../../../../../UIControl/data.qrc
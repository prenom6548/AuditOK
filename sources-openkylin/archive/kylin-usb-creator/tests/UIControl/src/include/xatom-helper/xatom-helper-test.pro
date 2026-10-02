TEMPLATE = app
TARGET = test_xatom-helper

# 添加包含路径
INCLUDEPATH += $$PWD \
               $$PWD/../../../../../UIControl/src/include \
               /usr/include/X11 \
               /usr/include/qt5 \
               /usr/include/qt5/QtCore \
               /usr/include/qt5/QtGui \
               /usr/include/qt5/QtTest \
               /usr/include/qt5/QtX11Extras \
               /usr/include/KF5/KWindowSystem

# 添加依赖的模块
QT += core gui testlib x11extras widgets

# 源文件
SOURCES += \
    main.cpp \
    test_xatom-helper.cpp \
    ../../../../../UIControl/src/include/xatom-helper.cpp

# 头文件
HEADERS += \
    test_xatom-helper.h \
    ../../../../../UIControl/src/include/xatom-helper.h

# 链接库
LIBS += -lX11 -lXext -lQt5Core -lQt5Gui -lQt5Test -lQt5X11Extras -lQt5Widgets -lKF5WindowSystem

# 添加覆盖率标志
QMAKE_CXXFLAGS += --coverage -fno-exceptions
QMAKE_LFLAGS += --coverage -lgcov

# 定义NEPTUNE_THEME标志以避免编译错误
DEFINES += NEPTUNE_THEME

# 添加Qt模块
QT += widgets gui core testlib x11extras KWindowSystem

# 确保自动生成moc文件
CONFIG += moc
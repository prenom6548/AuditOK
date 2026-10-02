TEMPLATE = app
TARGET = test_mydefine

# 添加Qt模块
QT += core widgets testlib dbus

# 包含路径
INCLUDEPATH += ../../.. ../../../common

# 源文件
SOURCES += \
    main.cpp \
    test_mydefine.cpp \
    ../../../common/mydefine.cpp

# 头文件
HEADERS += \
    test_mydefine.h \
    ../../../common/mydefine.h

# 编译器标志
QMAKE_CXXFLAGS += --coverage
QMAKE_LFLAGS += --coverage -lgcov

# 定义测试宏
DEFINES += UNIT_TEST

# 链接Qt测试库
LIBS += -lQt5Test -lQt5Core -lQt5Widgets -lQt5DBus
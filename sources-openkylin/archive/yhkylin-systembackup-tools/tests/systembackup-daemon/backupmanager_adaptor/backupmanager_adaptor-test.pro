TEMPLATE = app
TARGET = test_backupmanager_adaptor

# 添加Qt模块
QT += core testlib dbus

QMAKE_CXXFLAGS +=  -fno-exceptions

# 包含路径
INCLUDEPATH += ../../.. ../.. ../../common

# 源文件
SOURCES += \
    main.cpp \
    test_backupmanager_adaptor.cpp \
    ../../../systembackup-daemon/backupmanager_adaptor.cpp

# 头文件
HEADERS += \
    test_backupmanager_adaptor.h \
    ../../../systembackup-daemon/backupmanager_adaptor.h

# 编译器标志
QMAKE_CXXFLAGS += --coverage
QMAKE_LFLAGS += --coverage -lgcov

# 定义测试宏
DEFINES += UNIT_TEST

# 链接Qt测试库和其他所需库
LIBS += -lQt5Test -lQt5Core -lQt5Widgets -lQt5DBus -lblkid

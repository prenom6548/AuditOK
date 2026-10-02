TEMPLATE = app
TARGET = test_utils

# 添加Qt模块
QT += core testlib dbus xml

QMAKE_CXXFLAGS +=  -fno-exceptions

# 包含路径
INCLUDEPATH += ../../.. ../../../common ../../../systembackup-daemon

# 源文件
SOURCES += \
    main.cpp \
    test_utils.cpp \
    ../../../common/utils.cpp \
    ../../../common/mylittleparse.cpp \
    ../../../systembackup-daemon/parsebackuplist.cpp

# 头文件
HEADERS += \
    test_utils.h \
    ../../../common/utils.h \
    ../../../common/mydefine.h \
    ../../../common/mylittleparse.h \
    ../../../systembackup-daemon/parsebackuplist.h

# 编译器标志
QMAKE_CXXFLAGS += --coverage
QMAKE_LFLAGS += --coverage -lgcov

# 定义测试宏
DEFINES += UNIT_TEST

# 链接Qt测试库
LIBS += -lQt5Test -lQt5Core -lQt5DBus -lQt5Xml

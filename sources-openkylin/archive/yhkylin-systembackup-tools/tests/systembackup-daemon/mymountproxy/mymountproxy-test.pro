TEMPLATE = app
TARGET = test_mymountproxy

# 添加Qt模块
QT += core widgets testlib dbus xml

QMAKE_CXXFLAGS +=  -fno-exceptions

# 包含路径
INCLUDEPATH += ../../.. ../../../systembackup-daemon ../../../common ../../../systembackup-daemon/myprocess

# 源文件
SOURCES += \
    main.cpp \
    test_mymountproxy.cpp \
    ../../../systembackup-daemon/mymountproxy.cpp \
    ../../../systembackup-daemon/myprocess/mountbackupprocess.cpp \
    ../../../systembackup-daemon/parsebackuplist.cpp \
    ../../../common/utils.cpp \
    ../../../common/mydefine.cpp \
    ../../../common/mylittleparse.cpp \
    ../../../common/reflect.cpp

# 头文件
HEADERS += \
    test_mymountproxy.h \
    ../../../systembackup-daemon/mymountproxy.h \
    ../../../systembackup-daemon/myprocess/mountbackupprocess.h \
    ../../../systembackup-daemon/parsebackuplist.h \
    ../../../common/utils.h \
    ../../../common/mydefine.h \
    ../../../common/mylittleparse.h \
    ../../../common/reflect.h

# 编译器标志
QMAKE_CXXFLAGS += --coverage
QMAKE_LFLAGS += --coverage -lgcov

# 定义测试宏
DEFINES += UNIT_TEST

# 链接Qt测试库
LIBS += -lQt5Test -lQt5Core -lQt5Widgets -lQt5DBus -lQt5Xml

TEMPLATE = app
TARGET = test_customizedatabackupproxy
CONFIG += testcase

# 添加Qt模块
QT += core widgets testlib dbus xml

QMAKE_CXXFLAGS +=  -fno-exceptions

# 包含路径
INCLUDEPATH += ../../.. ../../../systembackup-daemon ../../../common

# 源文件
SOURCES += \
    main.cpp \
    test_customizedatabackupproxy.cpp \
    ../../../systembackup-daemon/customizedatabackupproxy.cpp \
    ../../../systembackup-daemon/workerfactory.cpp \
    ../../../systembackup-daemon/myprocess/calcbackupsize.cpp \
    ../../../systembackup-daemon/myprocess/rsyncpathtodirprocess.cpp \
    ../../../systembackup-daemon/myprocess/mountbackupprocess.cpp \
    ../../../systembackup-daemon/parsebackuplist.cpp \
    ../../../systembackup-daemon/mymountproxy.cpp \
    ../../../common/reflect.cpp \
    ../../../common/utils.cpp \
    ../../../common/mydusizetool.cpp \
    ../../../common/mylittleparse.cpp \
    ../../../common/mydefine.cpp

# 头文件
HEADERS += \
    test_customizedatabackupproxy.h \
    ../../../systembackup-daemon/customizedatabackupproxy.h \
    ../../../systembackup-daemon/workerfactory.h \
    ../../../systembackup-daemon/myprocess/calcbackupsize.h \
    ../../../systembackup-daemon/myprocess/rsyncpathtodirprocess.h \
    ../../../systembackup-daemon/myprocess/mountbackupprocess.h \
    ../../../systembackup-daemon/parsebackuplist.h \
    ../../../systembackup-daemon/mymountproxy.h \
    ../../../common/reflect.h \
    ../../../common/utils.h \
    ../../../common/mydusizetool.h \
    ../../../common/mylittleparse.h \
    ../../../common/mydefine.h

# 编译器标志
QMAKE_CXXFLAGS += --coverage
QMAKE_LFLAGS += --coverage -lgcov

# 定义测试宏
DEFINES += UNIT_TEST

# 链接Qt测试库
LIBS += -lQt5Test -lQt5Core -lQt5Widgets -lQt5DBus -lQt5Xml

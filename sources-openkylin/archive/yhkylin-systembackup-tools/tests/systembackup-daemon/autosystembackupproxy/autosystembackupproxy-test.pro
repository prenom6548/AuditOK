TEMPLATE = app
TARGET = test_autosystembackupproxy
CONFIG += testcase

# 添加Qt模块
QT += core widgets testlib dbus xml

QMAKE_CXXFLAGS +=  -fno-exceptions

# 包含路径
INCLUDEPATH += ../../.. ../../../systembackup-daemon ../../../common

# 源文件
SOURCES += \
    main.cpp \
    test_autosystembackupproxy.cpp \
    ../../../systembackup-daemon/autosystembackupproxy.cpp \
    ../../../systembackup-daemon/workerfactory.cpp \
    ../../../systembackup-daemon/myprocess/calcbackupsize.cpp \
    ../../../systembackup-daemon/myprocess/mksquashfsprocess.cpp \
    ../../../systembackup-daemon/myprocess/rsyncpathtodirprocess.cpp \
    ../../../systembackup-daemon/parsebackuplist.cpp \
    ../../../systembackup-daemon/myprocess/OstreeInterface.cpp \
    ../../../common/reflect.cpp \
    ../../../common/utils.cpp \
    ../../../common/mydusizetool.cpp \
    ../../../common/mylittleparse.cpp \
    ../../../common/mydefine.cpp

# 头文件
HEADERS += \
    test_autosystembackupproxy.h \
    ../../../systembackup-daemon/autosystembackupproxy.h \
    ../../../systembackup-daemon/workerfactory.h \
    ../../../systembackup-daemon/myprocess/calcbackupsize.h \
    ../../../systembackup-daemon/myprocess/mksquashfsprocess.h \
    ../../../systembackup-daemon/myprocess/rsyncpathtodirprocess.h \
    ../../../systembackup-daemon/parsebackuplist.h \
    ../../../systembackup-daemon/myprocess/OstreeInterface.h \
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

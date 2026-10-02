TEMPLATE = app
TARGET = test_udisksystembackupproxy

# 添加Qt模块
QT += core testlib dbus xml
CONFIG += testcase

QMAKE_CXXFLAGS +=  -fno-exceptions

# 包含路径
INCLUDEPATH += \
    ../../../ \
    ../../../systembackup-daemon \
    ../../../systembackup-daemon/myprocess \
    ../../../common

# 源文件
SOURCES += \
    main.cpp \
    test_udisksystembackupproxy.cpp \
    ../../../systembackup-daemon/udisksystembackupproxy.cpp \
    ../../../systembackup-daemon/myprocess/calcbackupsize.cpp \
    ../../../systembackup-daemon/myprocess/mksquashfsprocess.cpp \
    ../../../systembackup-daemon/myprocess/rsyncpathtodirprocess.cpp \
    ../../../systembackup-daemon/myprocess/mountbackupprocess.cpp \
    ../../../systembackup-daemon/parsebackuplist.cpp \
    ../../../systembackup-daemon/workerfactory.cpp \
    ../../../common/utils.cpp \
    ../../../common/mydusizetool.cpp \
    ../../../systembackup-daemon/mymountproxy.cpp \
    ../../../common/mylittleparse.cpp \
    ../../../common/reflect.cpp \
    ../../../common/mydefine.cpp

# 头文件
HEADERS += \
    test_udisksystembackupproxy.h \
    ../../../systembackup-daemon/udisksystembackupproxy.h \
    ../../../systembackup-daemon/myprocess/calcbackupsize.h \
    ../../../systembackup-daemon/myprocess/mksquashfsprocess.h \
    ../../../systembackup-daemon/myprocess/rsyncpathtodirprocess.h \
    ../../../systembackup-daemon/myprocess/mountbackupprocess.h \
    ../../../systembackup-daemon/parsebackuplist.h \
    ../../../systembackup-daemon/workerfactory.h \
    ../../../common/utils.h \
    ../../../common/mydusizetool.h \
    ../../../systembackup-daemon/mymountproxy.h \
    ../../../common/mylittleparse.h \
    ../../../common/reflect.h \
    ../../../common/mydefine.h

# 编译器标志
QMAKE_CXXFLAGS += --coverage
QMAKE_LFLAGS += --coverage -lgcov

DEFINES += UNIT_TEST

# 链接Qt测试库
LIBS += -lQt5Test -lQt5Core -lQt5DBus -lQt5Xml

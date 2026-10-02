TEMPLATE = app
TARGET = test_mybackupmanager

# 添加Qt模块
QT += core testlib dbus xml
CONFIG += testcase

QMAKE_CXXFLAGS +=  -fno-exceptions

# 包含路径
INCLUDEPATH += ../../.. ../.. ../../common ../../systembackup-daemon

# 源文件
SOURCES += \
    ../../../systembackup-daemon/autosystembackupproxy.cpp \
    ../../../systembackup-daemon/autosystemrestoreproxy.cpp \
    ../../../systembackup-daemon/customizedatabackupproxy.cpp \
    ../../../systembackup-daemon/customizeghostImageproxy.cpp \
    ../../../systembackup-daemon/customizesystembackupproxy.cpp \
    ../../../systembackup-daemon/databackupproxy.cpp \
    ../../../systembackup-daemon/datarestoreproxy.cpp \
    ../../../systembackup-daemon/deletebackupproxy.cpp \
    ../../../systembackup-daemon/ghostimageproxy.cpp \
    ../../../systembackup-daemon/myprocess/OstreeInterface.cpp \
    ../../../systembackup-daemon/myprocess/calcbackupsize.cpp \
    ../../../systembackup-daemon/myprocess/mksquashfsprocess.cpp \
    ../../../systembackup-daemon/myprocess/rsyncpathtodirprocess.cpp \
    ../../../systembackup-daemon/systembackupproxy.cpp \
    ../../../systembackup-daemon/systemrestoreproxy.cpp \
    ../../../systembackup-daemon/systemrestoreworker.cpp \
    ../../../systembackup-daemon/udisksystembackupproxy.cpp \
    ../../../systembackup-daemon/udisksystemrestoreproxy.cpp \
    main.cpp \
    test_mybackupmanager.cpp \
    ../../../systembackup-daemon/mybackupmanager.cpp \
    ../../../systembackup-daemon/mymountproxy.cpp \
    ../../../systembackup-daemon/mythread.cpp \
    ../../../systembackup-daemon/parsebackuplist.cpp \
    ../../../systembackup-daemon/translations.cpp \
    ../../../systembackup-daemon/workerfactory.cpp \
    ../../../systembackup-daemon/myprocess/mountbackupprocess.cpp \
    ../../../common/utils.cpp \
    ../../../common/reflect.cpp \
    ../../../common/mylittleparse.cpp \
    ../../../common/mydefine.cpp

# 头文件
HEADERS += \
    ../../../systembackup-daemon/autosystembackupproxy.h \
    ../../../systembackup-daemon/autosystemrestoreproxy.h \
    ../../../systembackup-daemon/customizedatabackupproxy.h \
    ../../../systembackup-daemon/customizeghostImageproxy.h \
    ../../../systembackup-daemon/customizesystembackupproxy.h \
    ../../../systembackup-daemon/databackupproxy.h \
    ../../../systembackup-daemon/datarestoreproxy.h \
    ../../../systembackup-daemon/deletebackupproxy.h \
    ../../../systembackup-daemon/ghostimageproxy.h \
    ../../../systembackup-daemon/myprocess/OstreeInterface.h \
    ../../../systembackup-daemon/myprocess/calcbackupsize.h \
    ../../../systembackup-daemon/myprocess/mksquashfsprocess.h \
    ../../../systembackup-daemon/myprocess/rsyncpathtodirprocess.h \
    ../../../systembackup-daemon/systembackupproxy.h \
    ../../../systembackup-daemon/systemrestoreproxy.h \
    ../../../systembackup-daemon/systemrestoreworker.h \
    ../../../systembackup-daemon/udisksystembackupproxy.h \
    ../../../systembackup-daemon/udisksystemrestoreproxy.h \
    test_mybackupmanager.h \
    ../../../systembackup-daemon/mybackupmanager.h \
    ../../../systembackup-daemon/mymountproxy.h \
    ../../../systembackup-daemon/mythread.h \
    ../../../systembackup-daemon/parsebackuplist.h \
    ../../../systembackup-daemon/translations.h \
    ../../../systembackup-daemon/workerfactory.h \
    ../../../systembackup-daemon/myprocess/mountbackupprocess.h \
    ../../../common/utils.h \
    ../../../common/mydefine.h \
    ../../../common/spinlock_mutex.h \
    ../../../common/singleton.h \
    ../../../common/reflect.h \
    ../../../common/mylittleparse.h

# 编译器标志
QMAKE_CXXFLAGS += --coverage
QMAKE_LFLAGS += --coverage -lgcov

# 定义测试宏
DEFINES += UNIT_TEST

# 链接Qt测试库和其他所需库
LIBS += -lQt5Test -lQt5Core -lQt5Widgets -lQt5DBus -lQt5Xml -lblkid

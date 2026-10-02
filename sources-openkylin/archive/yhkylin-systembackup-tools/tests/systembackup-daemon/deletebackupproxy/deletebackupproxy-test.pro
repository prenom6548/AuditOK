TEMPLATE = app
TARGET = test_deletebackupproxy

QT += testlib
CONFIG += testcase

QMAKE_CXXFLAGS +=  -fno-exceptions

SOURCES += \
    test_deletebackupproxy.cpp \
    main.cpp

HEADERS += \
    test_deletebackupproxy.h

INCLUDEPATH += \
    ../../../systembackup-daemon \
    ../../../systembackup-daemon/myprocess \
    ../../../common

# 添加被测源代码文件
SOURCES += \
    ../../../systembackup-daemon/deletebackupproxy.cpp \
    ../../../systembackup-daemon/parsebackuplist.cpp \
    ../../../systembackup-daemon/myprocess/OstreeInterface.cpp \
    ../../../systembackup-daemon/workerfactory.cpp \
    ../../../common/utils.cpp \
    ../../../common/reflect.cpp \
    ../../../common/mylittleparse.cpp \
    ../../../common/mydefine.cpp

HEADERS += \
    ../../../systembackup-daemon/deletebackupproxy.h \
    ../../../systembackup-daemon/parsebackuplist.h \
    ../../../systembackup-daemon/myprocess/OstreeInterface.h \
    ../../../systembackup-daemon/workerfactory.h \
    ../../../common/utils.h \
    ../../../common/reflect.h \
    ../../../common/mylittleparse.h \
    ../../../common/mydefine.h \
    ../../../common/dynamiccreator.h \
    ../../../common/singleton.h \
    ../../../common/spinlock_mutex.h

# 添加覆盖率标志
QMAKE_CXXFLAGS += --coverage
QMAKE_LDFLAGS += --coverage
LIBS += -lgcov

# 链接 Qt5 模块
QT += core widgets network dbus xml

# 如果需要链接其他库，可以在这里添加
# LIBS += -L../../../systembackup-daemon -lyourlib

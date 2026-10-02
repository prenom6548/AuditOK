TEMPLATE = app
TARGET = test_autosystemrestoreproxy

# 添加Qt模块
QT += core widgets testlib dbus xml

# 包含路径
INCLUDEPATH += ../../.. ../../../systembackup-daemon ../../../common

# 源文件
SOURCES += \
    main.cpp \
    test_autosystemrestoreproxy.cpp \
    ../../../systembackup-daemon/autosystemrestoreproxy.cpp \
    ../../../systembackup-daemon/systemrestoreworker.cpp \
    ../../../systembackup-daemon/workerfactory.cpp \
    ../../../systembackup-daemon/myprocess/OstreeInterface.cpp \
    ../../../systembackup-daemon/myprocess/rsyncpathtodirprocess.cpp \
    ../../../systembackup-daemon/myprocess/mountbackupprocess.cpp \
    ../../../systembackup-daemon/parsebackuplist.cpp \
    ../../../systembackup-daemon/translations.cpp \
    ../../../common/utils.cpp \
    ../../../common/mydusizetool.cpp \
    ../../../common/mylittleparse.cpp \
    ../../../common/reflect.cpp

# 头文件
HEADERS += \
    test_autosystemrestoreproxy.h \
    ../../../systembackup-daemon/autosystemrestoreproxy.h \
    ../../../systembackup-daemon/systemrestoreworker.h \
    ../../../systembackup-daemon/workerfactory.h \
    ../../../systembackup-daemon/myprocess/OstreeInterface.h \
    ../../../systembackup-daemon/myprocess/rsyncpathtodirprocess.h \
    ../../../systembackup-daemon/myprocess/mountbackupprocess.h \
    ../../../systembackup-daemon/parsebackuplist.h \
    ../../../systembackup-daemon/translations.h \
    ../../../common/utils.h \
    ../../../common/mydusizetool.h \
    ../../../common/mylittleparse.h \
    ../../../common/reflect.h \
    ../../../common/mydefine.h \
    ../../../common/dynamiccreator.h

# 编译器标志
QMAKE_CXXFLAGS += --coverage
QMAKE_LFLAGS += --coverage -lgcov

# 定义测试宏
DEFINES += UNIT_TEST

# 链接Qt测试库
LIBS += -lQt5Test -lQt5Core -lQt5Widgets -lQt5DBus -lQt5Xml
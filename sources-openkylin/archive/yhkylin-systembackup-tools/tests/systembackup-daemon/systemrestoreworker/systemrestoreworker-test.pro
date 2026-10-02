TEMPLATE = app
TARGET = test_systemrestoreworker

# 添加Qt模块
QT -= gui  # 移除GUI模块以减少依赖
QT += dbus xml testlib  # 添加DBus、XML和Test模块

# 包含路径
INCLUDEPATH += . \
    ../../../systembackup-daemon \
    ../../../systembackup-daemon/myprocess \
    ../../../common

# 源文件
SOURCES += main.cpp \
           test_systemrestoreworker.cpp \
           ../../../systembackup-daemon/systemrestoreworker.cpp \
           ../../../systembackup-daemon/workerfactory.cpp \
           ../../../systembackup-daemon/myprocess/rsyncpathtodirprocess.cpp \
           ../../../systembackup-daemon/parsebackuplist.cpp \
           ../../../systembackup-daemon/translations.cpp \
           ../../../common/utils.cpp \
           ../../../common/mydusizetool.cpp \
           ../../../common/reflect.cpp \
           ../../../common/mylittleparse.cpp \
           ../../../common/mydefine.cpp

# 头文件
HEADERS += test_systemrestoreworker.h \
           ../../../systembackup-daemon/systemrestoreworker.h \
           ../../../systembackup-daemon/workerfactory.h \
           ../../../systembackup-daemon/myprocess/rsyncpathtodirprocess.h \
           ../../../systembackup-daemon/parsebackuplist.h \
           ../../../systembackup-daemon/translations.h \
           ../../../common/utils.h \
           ../../../common/mydusizetool.h \
           ../../../common/reflect.h \
           ../../../common/mylittleparse.h \
           ../../../common/mydefine.h \
           ../../../common/singleton.h \
           ../../../common/spinlock_mutex.h

# 编译器标志
QMAKE_CXXFLAGS += --coverage -fpermissive
QMAKE_LFLAGS += --coverage

# 链接库
LIBS += -lQt5Core -lQt5Test -lQt5DBus -lQt5Xml -lgcov -lblkid
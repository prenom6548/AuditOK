TEMPLATE = app
TARGET = test_translations

# 添加Qt模块
QT += core testlib dbus

QMAKE_CXXFLAGS +=  -fno-exceptions

# 包含路径
INCLUDEPATH += ../../.. ../../../systembackup-daemon ../../../common

# 源文件
SOURCES += \
    main.cpp \
    test_translations.cpp \
    ../../../systembackup-daemon/translations.cpp

# 头文件
HEADERS += \
    test_translations.h \
    ../../../systembackup-daemon/translations.h \
    ../../../common/singleton.h

# 编译器标志
QMAKE_CXXFLAGS += --coverage
QMAKE_LFLAGS += --coverage -lgcov

# 定义测试宏
DEFINES += UNIT_TEST

# 链接Qt测试库
LIBS += -lQt5Test -lQt5Core

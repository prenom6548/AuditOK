TEMPLATE = app
TARGET = test_mylittleparse

# 添加Qt模块
QT += core testlib

QMAKE_CXXFLAGS +=  -fno-exceptions

# 包含路径
INCLUDEPATH += ../../.. ../../../common

# 源文件
SOURCES += \
    main.cpp \
    test_mylittleparse.cpp \
    ../../../common/mylittleparse.cpp

# 头文件
HEADERS += \
    test_mylittleparse.h \
    ../../../common/mylittleparse.h

# 编译器标志（可选：覆盖率）
QMAKE_CXXFLAGS += --coverage
QMAKE_LFLAGS += --coverage -lgcov

# 定义测试宏
DEFINES += UNIT_TEST

# 链接Qt测试库
LIBS += -lQt5Test -lQt5Core

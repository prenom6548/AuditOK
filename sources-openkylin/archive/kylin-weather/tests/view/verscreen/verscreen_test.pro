QT += core gui testlib widgets
TEMPLATE = app

TARGET = test_verscreen

# 添加覆盖率标志
QMAKE_CXXFLAGS += --coverage -fno-exceptions -fbranch-count-reg
QMAKE_LFLAGS += --coverage -lgcov

# 包含源文件
SOURCES += \
    test_verscreen.cpp \
    ../../../view/verscreen/verscreen.cpp

# 包含头文件
HEADERS += \
    test_verscreen.h \
    ../../../view/verscreen/verscreen.h

# 包含路径
INCLUDEPATH += ../../../view/verscreen ../../../global ../../../controller ../../../model ../..

# 链接Qt模块
QT += widgets testlib
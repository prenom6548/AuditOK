QT += testlib widgets gui core
TEMPLATE = app
TARGET = test_theme

# 包含被测源代码目录
INCLUDEPATH += ../../../../../UIControl/src

# 包含被测源代码文件
SOURCES += \
    test_theme.cpp \
    main.cpp \
    ../../../../../UIControl/src/include/theme.cpp

HEADERS += \
    test_theme.h \
    ../../../../../UIControl/src/include/theme.h

# 添加覆盖率标志
QMAKE_CXXFLAGS += --coverage -fno-exceptions
QMAKE_LFLAGS += --coverage -lgcov

# 链接所需库
LIBS += -lgsettings-qt

# 添加Qt模块
QT += widgets gui core testlib

# 设置包含路径
INCLUDEPATH += ../../../../../UIControl/src/include

# 设置资源文件（暂时注释掉，因为我们不需要测试资源文件）
# RESOURCES += ../../../../../UIControl/data.qrc
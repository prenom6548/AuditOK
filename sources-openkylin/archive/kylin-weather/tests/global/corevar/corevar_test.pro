QT += core gui testlib widgets
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt
TEMPLATE = app

TARGET = test_corevar

# 添加覆盖率标志
QMAKE_CXXFLAGS += --coverage -fno-exceptions -fbranch-count-reg
QMAKE_LFLAGS += --coverage -lgcov

# 包含源文件
SOURCES += \
    test_corevar.cpp \
    main.cpp \
    ../../../global/corevar.cpp

# 包含头文件
HEADERS += \
    test_corevar.h \
    ../../../global/corevar.h

# 包含路径
INCLUDEPATH += ../../../global

QT += core gui testlib widgets dbus
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt
TEMPLATE = app

TARGET = test_rotatechangeinfo

# 添加覆盖率标志
QMAKE_CXXFLAGS += --coverage -fno-exceptions -fbranch-count-reg
QMAKE_LFLAGS += --coverage -lgcov

# 包含源文件
SOURCES += \
    test_rotatechangeinfo.cpp \
    main.cpp \
    ../../../global/rotatechangeinfo.cpp

# 包含头文件
HEADERS += \
    test_rotatechangeinfo.h \
    ../../../global/rotatechangeinfo.h

# 包含路径
INCLUDEPATH += ../../../global
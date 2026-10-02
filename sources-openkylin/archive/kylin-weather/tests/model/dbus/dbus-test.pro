QT += core testlib dbus network gui widgets
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt
TEMPLATE = app

TARGET = test_dbus

# 添加覆盖率标志
QMAKE_CXXFLAGS += --coverage -fno-exceptions -fbranch-count-reg
QMAKE_LFLAGS += --coverage -lgcov

# 添加 Google Mock
LIBS += -lgmock -lgtest -lpthread

# 包含源文件
SOURCES += \
    test_dbus.cpp \
    main.cpp \
    ../../../model/dbus.cpp

# 包含头文件
HEADERS += \
    test_dbus.h \
    ../../../model/dbus.h

# 包含路径 - 必须包含项目根目录以解析 "global/corevar.h" 这种格式的包含
INCLUDEPATH += ../../../ \
               ../../../model \
               ../../../controller \
               ../../../controller/core \
               ../../../global

QT += core gui testlib widgets dbus
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt kysdk-kabase kysdk-sysinfo
TEMPLATE = app

TARGET = test_bottomlistdelegate

# 添加覆盖率标志
QMAKE_CXXFLAGS += --coverage -fno-exceptions -fbranch-count-reg
QMAKE_LFLAGS += --coverage -lgcov

# 添加kysdk库和包含路径
LIBS += -lkylog -lkyconf
INCLUDEPATH += /usr/include/kysdk/applications/kabase/ /usr/include/kysdk/applications/ /usr/include/kysdk/applications/kabase/kylin_system/

# 包含源文件
SOURCES += \
    test_bottomlistdelegate.cpp \
    main.cpp \
    ../../../../view/search/bottomlistdelegate.cpp \
    ../../../../model/titlesearchmodel.cpp \
    ../../../../global/corevar.cpp \
    ../../../../global/viewvar.cpp \
    ../../../../global/rotatechangeinfo.cpp

# 包含头文件
HEADERS += \
    test_bottomlistdelegate.h \
    ../../../../view/search/bottomlistdelegate.h \
    ../../../../model/titlesearchmodel.h \
    ../../../../global/corevar.h \
    ../../../../global/viewvar.h \
    ../../../../global/rotatechangeinfo.h

# 包含路径
INCLUDEPATH += ../../../../view/search ../../../../model ../../../../global ../../../..
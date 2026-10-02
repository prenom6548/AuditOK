QT += core gui testlib widgets network dbus
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt kysdk-kabase kysdk-datacollect
TEMPLATE = app

TARGET = test_hotcity

# 添加覆盖率标志
QMAKE_CXXFLAGS += --coverage -fno-exceptions -fbranch-count-reg
QMAKE_LFLAGS += --coverage -lgcov

# 添加kysdk库和包含路径
LIBS += -lkylog -lkydiagnostics -lkyconf -lsystemd -lkydatastruct -lkyutils
INCLUDEPATH += /usr/include/kysdk/kysdk-base/ /usr/include/kysdk/applications/kabase/ /usr/include/kysdk/applications/kabase/kylin_system/ /usr/include/kysdk/applications/ /usr/include/kysdk/applications/ukuiwindowhelper/ /usr/include/KF5/KWindowSystem/ ../../../../

# 包含源文件
SOURCES += \
    main.cpp \
    test_hotcity.cpp \
    ../../../../view/search/hotcity.cpp \
    ../../../../view/search/hotcitybtn.cpp \
    ../../../../global/viewvar.cpp \
    ../../../../global/corevar.cpp \
    ../../../../global/rotatechangeinfo.cpp

# 包含头文件
HEADERS += \
    test_hotcity.h \
    ../../../../view/search/hotcity.h \
    ../../../../view/search/hotcitybtn.h \
    ../../../../global/viewvar.h \
    ../../../../global/corevar.h \
    ../../../../global/rotatechangeinfo.h

# 包含路径
INCLUDEPATH += .. ../../../../model ../../../../controller/core ../../../../global ../../../../kabase ../../../../kabase/Qt ../../../../view/search ../../../../controller ../../../../view/horscreen
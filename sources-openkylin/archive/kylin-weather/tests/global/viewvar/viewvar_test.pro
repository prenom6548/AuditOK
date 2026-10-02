QT += core gui testlib widgets dbus
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt kysdk-kabase
TEMPLATE = app

TARGET = test_viewvar

# 添加覆盖率标志
QMAKE_CXXFLAGS += --coverage -fno-exceptions -fbranch-count-reg
QMAKE_LFLAGS += --coverage -lgcov

# 添加kysdk库和包含路径
LIBS += -lkylog -lkydiagnostics -lkyconf -lsystemd -lkydatastruct -lkyutils
INCLUDEPATH += /usr/include/kysdk/kysdk-base/ /usr/include/kysdk/applications/kabase/ /usr/include/kysdk/applications/kabase/kylin_system/ /usr/include/kysdk/applications/ /usr/include/kysdk/applications/ukuiwindowhelper/ /usr/include/KF5/KWindowSystem/

# 包含源文件
SOURCES += \
    test_viewvar.cpp \
    main.cpp \
    ../../../global/viewvar.cpp \
    ../../../global/rotatechangeinfo.cpp

# 包含头文件
HEADERS += \
    test_viewvar.h \
    ../../../global/viewvar.h \
    ../../../global/rotatechangeinfo.h

# 包含路径
INCLUDEPATH += ../../../global
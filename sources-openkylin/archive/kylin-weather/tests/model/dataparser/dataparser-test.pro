QT += core gui testlib widgets network
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt
TEMPLATE = app

TARGET = test_dataparser

# 添加覆盖率标志
QMAKE_CXXFLAGS += --coverage -fno-exceptions -fbranch-count-reg
QMAKE_LFLAGS += --coverage -lgcov

# 添加kysdk库和包含路径
LIBS += -lkylog -lkydiagnostics -lkyconf -lsystemd -lkydatastruct -lkyutils
INCLUDEPATH += /usr/include/kysdk/kysdk-base/ /usr/include/kysdk/applications/kabase/ /usr/include/kysdk/applications/kabase/kylin_system/ /usr/include/kysdk/applications/ /usr/include/kysdk/applications/ukuiwindowhelper/ /usr/include/KF5/KWindowSystem/ .. ../../../

# 包含源文件
SOURCES += \
    test_dataparser.cpp \
    main.cpp \
    ../../../model/dataparser.cpp \
    ../../../global/corevar.cpp \
    ../../../controller/core/errorreact.cpp

# 包含头文件
HEADERS += \
    test_dataparser.h \
    ../../../model/dataparser.h \
    ../../../global/corevar.h \
    ../../../controller/core/errorreact.h

# 包含路径
INCLUDEPATH += ../../../model ../../../controller/core ../../../global ../../../kabase ../../../kabase/Qt
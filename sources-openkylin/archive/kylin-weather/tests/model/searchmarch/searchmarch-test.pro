QT += core gui testlib widgets network
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt
TEMPLATE = app

TARGET = test_searchmarch

# 添加覆盖率标志
QMAKE_CXXFLAGS += --coverage -fno-exceptions -fbranch-count-reg
QMAKE_LFLAGS += --coverage -lgcov

# 添加kysdk库和包含路径
LIBS += -lkylog -lkydiagnostics -lkyconf -lsystemd -lkydatastruct -lkyutils
INCLUDEPATH += /usr/include/kysdk/kysdk-base/ /usr/include/kysdk/applications/kabase/ /usr/include/kysdk/applications/kabase/kylin_system/ /usr/include/kysdk/applications/ /usr/include/kysdk/applications/ukuiwindowhelper/ /usr/include/KF5/KWindowSystem/ .. ../../../

# 包含源文件
SOURCES += \
    test_searchmarch.cpp \
    main.cpp \
    ../../../model/searchmarch.cpp \
    ../../../global/corevar.cpp

# 包含头文件
HEADERS += \
    test_searchmarch.h \
    ../../../model/searchmarch.h \
    ../../../global/corevar.h

# 包含资源文件
RESOURCES += \
    ../../../data.qrc

# 包含路径
INCLUDEPATH += ../../../model ../../../global ../../../controller/core ../../../kabase ../../../kabase/Qt
QT += core gui testlib widgets
CONFIG += link_pkgconfig
TEMPLATE = app

TARGET = test_titlesearchmodel

# 添加覆盖率标志
QMAKE_CXXFLAGS += --coverage -fno-exceptions -fbranch-count-reg
QMAKE_LFLAGS += --coverage -lgcov

# 添加kysdk库和包含路径
LIBS += -lkylog -lkydiagnostics -lkyconf -lsystemd -lkydatastruct -lkyutils
INCLUDEPATH += /usr/include/kysdk/kysdk-base/ /usr/include/kysdk/applications/kabase/ /usr/include/kysdk/applications/kabase/kylin_system/ /usr/include/kysdk/applications/ /usr/include/kysdk/applications/ukuiwindowhelper/ /usr/include/KF5/KWindowSystem/ .. ../../../

# 包含源文件
SOURCES += \
    test_titlesearchmodel.cpp \
    main.cpp \
    ../../../model/titlesearchmodel.cpp

# 包含头文件
HEADERS += \
    test_titlesearchmodel.h \
    ../../../model/titlesearchmodel.h

# 包含资源文件
RESOURCES += \
    ../../../data.qrc

# 包含路径
INCLUDEPATH += ../../../model ../../../global ../../../controller/core ../../../kabase ../../../kabase/Qt
QT += core gui testlib widgets dbus network x11extras KWindowSystem
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt kysdk-kabase kysdk-ukuiwindowhelper kysdk-waylandhelper
TEMPLATE = app

TARGET = test_bottomlistview

# 添加覆盖率标志
QMAKE_CXXFLAGS += --coverage -fno-exceptions -fbranch-count-reg
QMAKE_LFLAGS += --coverage -lgcov

# 包含路径
INCLUDEPATH += . \
               ../../../.. \
               ../../../../view/search \
               ../../../../model \
               ../../../../global \
               ../../../../controller \
               ../../../../kabase \
               ../../../../kabase/Qt \
               /usr/include/kabase/ \
               /usr/include/kysdk/applications/ \
               /usr/include/kysdk/applications/kabase/ \
               /usr/include/kysdk/applications/kabase/kylin_system/

# 源文件
SOURCES += \
    main.cpp \
    test_bottomlistview.cpp \
    ../../../../view/search/bottomlistview.cpp \
    ../../../../view/search/bottomlistdelegate.cpp \
    ../../../../model/titlesearchmodel.cpp \
    ../../../../global/viewvar.cpp \
    ../../../../global/corevar.cpp \
    ../../../../global/rotatechangeinfo.cpp \
    mock_symbols.cpp

# 头文件
HEADERS += \
    test_bottomlistview.h \
    ../../../../view/search/bottomlistview.h \
    ../../../../view/search/bottomlistdelegate.h \
    ../../../../model/titlesearchmodel.h \
    ../../../../global/viewvar.h \
    ../../../../global/corevar.h \
    ../../../../global/rotatechangeinfo.h \
    ../../../../controller/Interaction.h

# 链接库
#LIBS += -lgtest -lgmock -lgcov --coverage
LIBS += -lkylog -lkyconf

QT += core gui testlib widgets dbus network x11extras KWindowSystem
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt kysdk-kabase kysdk-ukuiwindowhelper kysdk-waylandhelper
TEMPLATE = app

TARGET = test_titlesearchcityview

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

# 包含源文件
SOURCES += \
    main.cpp \
    test_titlesearchcityview.cpp \
    ../../../../view/search/titlesearchcityview.cpp \
    ../../../../view/search/titlesearchlistdelegate.cpp \
    ../../../../model/titlesearchmodel.cpp \
    ../../../../global/viewvar.cpp \
    ../../../../global/corevar.cpp \
    ../../../../global/rotatechangeinfo.cpp \
    mock_symbols.cpp

# 头文件
HEADERS += \
    test_titlesearchcityview.h \
    ../../../../view/search/titlesearchcityview.h \
    ../../../../view/search/titlesearchlistdelegate.h \
    ../../../../model/titlesearchmodel.h \
    ../../../../global/viewvar.h \
    ../../../../global/corevar.h \
    ../../../../global/rotatechangeinfo.h \
    ../../../../controller/Interaction.h

# 链接库
#LIBS += -lgtest -lgmock -lgcov --coverage
LIBS += -lkylog -lkyconf

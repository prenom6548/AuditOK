QT += core gui testlib widgets dbus
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt kysdk-kabase
TEMPLATE = app

TARGET = test_searchdelegate

# 添加覆盖率标志
QMAKE_CXXFLAGS += --coverage -fno-exceptions -fbranch-count-reg
QMAKE_LFLAGS += --coverage -lgcov

# 添加kysdk库和包含路径
LIBS += -lkylog -lkyconf
INCLUDEPATH += /usr/include/kysdk/applications/kabase/ /usr/include/kysdk/applications/ /usr/include/kysdk/applications/kabase/kylin_system/

# 包含路径 - 确保本地路径优先
INCLUDEPATH += . \
               ../../../.. \
               ../../../../view/search \
               ../../../../model \
               ../../../../global \
               ../../../../controller \
               ../../../../kabase \
               ../../../../kabase/Qt

# 源文件
SOURCES += \
    test_searchdelegate.cpp \
    main.cpp \
    ../../../../view/search/searchdelegate.cpp \
    ../../../../global/corevar.cpp \
    ../../../../global/viewvar.cpp \
    ../../../../global/rotatechangeinfo.cpp \
    mock_symbols.cpp

# 头文件
HEADERS += \
    test_searchdelegate.h \
    ../../../../view/search/searchdelegate.h \
    ../../../../global/corevar.h \
    ../../../../global/viewvar.h \
    ../../../../global/rotatechangeinfo.h \
    ../../../../controller/Interaction.h

# 链接库
#LIBS += -lgtest -lgmock -lgcov --coverage

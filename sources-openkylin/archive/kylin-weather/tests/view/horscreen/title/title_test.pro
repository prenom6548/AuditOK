QT += core gui widgets testlib dbus network x11extras KWindowSystem
CONFIG += link_pkgconfig
PKGCONFIG += kysdk-qtwidgets kysdk-log kysdk-waylandhelper kysdk-kabase gsettings-qt kysdk-datacollect kysdk-ukuiwindowhelper
TEMPLATE = app
TARGET = test_title

# 添加覆盖率标志
QMAKE_CXXFLAGS += --coverage -fno-exceptions -fbranch-count-reg
QMAKE_LFLAGS += --coverage -lgcov

# 添加kysdk库和包含路径
LIBS += -lkylog -lkyconf
INCLUDEPATH += /usr/include/kabase/ /usr/include/kysdk/applications/kabase/ /usr/include/kysdk/applications/kabase/kylin_system/ /usr/include/kysdk/applications/

# 定义包含路径 - 优先包含项目根目录
INCLUDEPATH += ../../../.. \
               ../../../../view/horscreen \
               ../../../../view/search \
               ../../../../controller \
               ../../../../model \
               ../../../../global \
               ../../../../kabase \
               ../../../../kabase/Qt

# 源文件
SOURCES += test_title.cpp \
           main.cpp \
           ../../../../view/horscreen/title.cpp \
           mock_symbols.cpp

# 头文件 - 包含所有涉及 Q_OBJECT 的头文件
HEADERS += test_title.h \
           ../../../../view/horscreen/title.h \
           ../../../../global/corevar.h \
           ../../../../global/viewvar.h \
           ../../../../controller/Interaction.h \
           ../../../../view/horscreen/horscreen.h \
           ../../../../view/horscreen/menumodule.h \
           ../../../../view/search/searchview.h

# 链接必要的库
#LIBS += -lgtest -lgmock -lgcov --coverage

# 添加MOC相关配置
CONFIG += moc

# Google Mock配置
CONFIG += gmock

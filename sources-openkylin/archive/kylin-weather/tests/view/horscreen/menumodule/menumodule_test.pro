QT += core gui testlib widgets dbus network x11extras KWindowSystem
CONFIG += link_pkgconfig
PKGCONFIG += kysdk-qtwidgets kysdk-log kysdk-waylandhelper kysdk-kabase gsettings-qt kysdk-datacollect kysdk-ukuiwindowhelper
TEMPLATE = app
TARGET = test_menumodule

# 添加覆盖率标志
QMAKE_CXXFLAGS += --coverage -fno-exceptions -fbranch-count-reg
QMAKE_LFLAGS += --coverage -lgcov

# 添加kysdk库和包含路径
LIBS += -lkylog -lkyconf
INCLUDEPATH += /usr/include/kabase/ /usr/include/kysdk/applications/kabase/ /usr/include/kysdk/applications/kabase/kylin_system/ /usr/include/kysdk/applications/

# 包含源文件
SOURCES += \
    test_menumodule.cpp \
    main.cpp \
    ../../../../view/horscreen/menumodule.cpp \
    mock_symbols.cpp

# 包含头文件
HEADERS += \
    test_menumodule.h \
    ../../../../view/horscreen/menumodule.h \
    ../../../../controller/Interaction.h \
    ../../../../global/corevar.h \
    ../../../../global/viewvar.h \
    ../../../../controller/core/viewcontrol.h

# 包含路径
INCLUDEPATH += ../../../../view/horscreen ../../../../controller ../../../../global ../../../../kabase ../../../../kabase/Qt ../../../.. /usr/include/kysdk/applications/ukuiwindowhelper/

# 添加MOC相关配置
CONFIG += moc

# Google Mock/Test 配置
CONFIG += gmock gtest
#LIBS += -lgmock -lgtest -lpthread

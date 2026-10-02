QT += core gui widgets testlib dbus network x11extras KWindowSystem
CONFIG += link_pkgconfig
PKGCONFIG += kysdk-qtwidgets kysdk-log kysdk-waylandhelper kysdk-kabase gsettings-qt kysdk-datacollect kysdk-ukuiwindowhelper
TEMPLATE = app

TARGET = test_horscreen

# 添加覆盖率标志
QMAKE_CXXFLAGS += --coverage -fno-exceptions -fbranch-count-reg
QMAKE_LFLAGS += --coverage -lgcov

# 添加kysdk库和包含路径
LIBS += -lkylog -lkyconf
INCLUDEPATH += /usr/include/kabase/ /usr/include/kysdk/applications/kabase/ /usr/include/kysdk/applications/kabase/kylin_system/ /usr/include/kysdk/applications/

SOURCES += \
    test_horscreen.cpp \
    main.cpp \
    ../../../../view/horscreen/horscreen.cpp \
    ../../../../view/horscreen/title.cpp \
    ../../../../view/horscreen/empty.cpp \
    ../../../../view/horscreen/city.cpp \
    ../../../../view/horscreen/information.cpp \
    ../../../../view/horscreen/onedayweather.cpp \
    ../../../../view/horscreen/perhour.cpp \
    ../../../../view/horscreen/weekweather.cpp \
    ../../../../view/horscreen/onehourweather.cpp \
    ../../../../view/horscreen/lifestyle.cpp \
    ../../../../view/horscreen/air.cpp \
    ../../../../view/horscreen/clothe.cpp \
    ../../../../view/horscreen/flu.cpp \
    ../../../../view/horscreen/sport.cpp \
    ../../../../view/horscreen/washcar.cpp \
    ../../../../view/horscreen/ultravioletrays.cpp \
    ../../../../view/horscreen/menumodule.cpp \
    ../../../../view/horscreen/promptwidget.cpp \
    ../../../../view/horscreen/allowlocation.cpp \
    ../../../../view/horscreen/splitline.cpp \
    ../../../../view/horscreen/nonetworkwidget.cpp \
    ../../../../view/horscreen/changecitywidget.cpp \
    mock_symbols.cpp

HEADERS += \
    test_horscreen.h \
    ../../../../view/horscreen/horscreen.h \
    ../../../../view/horscreen/title.h \
    ../../../../view/horscreen/empty.h \
    ../../../../view/horscreen/city.h \
    ../../../../view/horscreen/information.h \
    ../../../../view/horscreen/onedayweather.h \
    ../../../../view/horscreen/perhour.h \
    ../../../../view/horscreen/weekweather.h \
    ../../../../view/horscreen/onehourweather.h \
    ../../../../view/horscreen/lifestyle.h \
    ../../../../view/horscreen/air.h \
    ../../../../view/horscreen/clothe.h \
    ../../../../view/horscreen/flu.h \
    ../../../../view/horscreen/sport.h \
    ../../../../view/horscreen/washcar.h \
    ../../../../view/horscreen/ultravioletrays.h \
    ../../../../view/horscreen/menumodule.h \
    ../../../../view/horscreen/promptwidget.h \
    ../../../../view/horscreen/allowlocation.h \
    ../../../../view/horscreen/splitline.h \
    ../../../../view/horscreen/nonetworkwidget.h \
    ../../../../view/horscreen/changecitywidget.h \
    ../../../../view/search/searchview.h \
    ../../../../view/search/titlesearchcityview.h \
    ../../../../view/search/bottomlistview.h \
    ../../../../view/search/searchcitylist.h \
    ../../../../view/search/collectcity.h \
    ../../../../view/search/titlesearchlistdelegate.h \
    ../../../../view/search/bottomlistdelegate.h \
    ../../../../view/search/collectitemdelegate.h \
    ../../../../view/search/searchdelegate.h \
    ../../../../model/titlesearchmodel.h \
    ../../../../controller/Interaction.h \
    ../../../../global/corevar.h \
    ../../../../global/viewvar.h \
    ../../../../model/dataparser.h \
    ../../../../controller/core/viewcontrol.h \
    ../../../../global/rotatechangeinfo.h \
    ../../../../controller/core/core.h \
    ../../../../controller/core/errorreact.h \
    ../../../../controller/core/coreinteraction.h

INCLUDEPATH += ../../../../ \
    ../../../../view/horscreen/ \
    ../../../../view/search/ \
    ../../../../controller/ \
    ../../../../global/ \
    ../../../../model/ \
    ../../../../kabase/ \
    ../../../../kabase/Qt/

LIBS += -lgcov --coverage

# 添加MOC相关配置
CONFIG += moc

# Google Mock/Test 配置
CONFIG += gmock gtest
LIBS += -lgmock -lgtest -lpthread

# 不再显式添加MOC文件到目标

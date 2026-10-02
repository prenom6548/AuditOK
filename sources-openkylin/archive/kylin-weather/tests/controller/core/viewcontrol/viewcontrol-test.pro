QT += core gui testlib widgets dbus network x11extras KWindowSystem
CONFIG += link_pkgconfig c++11
TEMPLATE = app

TARGET = test_viewcontrol

# 添加覆盖率标志
QMAKE_CXXFLAGS += --coverage -fno-exceptions -fbranch-count-reg
QMAKE_LFLAGS += --coverage -lgcov

# 包含源文件
SOURCES += \
    test_viewcontrol.cpp \
    main.cpp \
    ../../../../controller/core/viewcontrol.cpp \
    ../../../../global/viewvar.cpp \
    ../../../../controller/core/coreinteraction.cpp \
    ../../../../controller/core/core.cpp \
    ../../../../controller/core/errorreact.cpp \
    ../../../../global/corevar.cpp \
    ../../../../global/rotatechangeinfo.cpp \
    ../../../../model/dataparser.cpp \
    ../../../../model/datarequest.cpp \
    ../../../../model/dbus.cpp \
    ../../../../model/iplocation.cpp \
    ../../../../model/network.cpp \
    ../../../../model/searchmarch.cpp \
    ../../../../model/titlesearchmodel.cpp \
    ../../../../view/horscreen/horscreen.cpp \
    ../../../../view/horscreen/air.cpp \
    ../../../../view/horscreen/allowlocation.cpp \
    ../../../../view/horscreen/city.cpp \
    ../../../../view/horscreen/clothe.cpp \
    ../../../../view/horscreen/empty.cpp \
    ../../../../view/horscreen/flu.cpp \
    ../../../../view/horscreen/information.cpp \
    ../../../../view/horscreen/lifestyle.cpp \
    ../../../../view/horscreen/menumodule.cpp \
    ../../../../view/horscreen/nonetworkwidget.cpp \
    ../../../../view/horscreen/onedayweather.cpp \
    ../../../../view/horscreen/onehourweather.cpp \
    ../../../../view/horscreen/perhour.cpp \
    ../../../../view/horscreen/promptwidget.cpp \
    ../../../../view/horscreen/splitline.cpp \
    ../../../../view/horscreen/sport.cpp \
    ../../../../view/horscreen/title.cpp \
    ../../../../view/horscreen/ultravioletrays.cpp \
    ../../../../view/horscreen/washcar.cpp \
    ../../../../view/horscreen/weekweather.cpp \
    ../../../../view/horscreen/changecitywidget.cpp \
    ../../../../view/horscreen/picturetowhite.cpp \
    ../../../../view/search/searchview.cpp \
    ../../../../view/search/collectcity.cpp \
    ../../../../view/search/collectitemdelegate.cpp \
    ../../../../view/search/hotcity.cpp \
    ../../../../view/search/hotcitybtn.cpp \
    ../../../../view/search/searchcitylist.cpp \
    ../../../../view/search/searchdelegate.cpp \
    ../../../../view/search/titlesearchcityview.cpp \
    ../../../../view/search/titlesearchlistdelegate.cpp \
    ../../../../view/search/bottomlistview.cpp \
    ../../../../view/search/bottomlistdelegate.cpp \
    ../../../../view/verscreen/verscreen.cpp

# 包含头文件
HEADERS += \
    ../../../../controller/Interaction.h \
    test_viewcontrol.h \
    ../../../../controller/core/viewcontrol.h \
    ../../../../global/viewvar.h \
    ../../../../controller/core/coreinteraction.h \
    ../../../../controller/core/core.h \
    ../../../../controller/core/errorreact.h \
    ../../../../global/corevar.h \
    ../../../../global/rotatechangeinfo.h \
    ../../../../model/dataparser.h \
    ../../../../model/datarequest.h \
    ../../../../model/dbus.h \
    ../../../../model/iplocation.h \
    ../../../../model/network.h \
    ../../../../model/searchmarch.h \
    ../../../../model/titlesearchmodel.h \
    ../../../../view/horscreen/horscreen.h \
    ../../../../view/horscreen/air.h \
    ../../../../view/horscreen/allowlocation.h \
    ../../../../view/horscreen/city.h \
    ../../../../view/horscreen/clothe.h \
    ../../../../view/horscreen/empty.h \
    ../../../../view/horscreen/flu.h \
    ../../../../view/horscreen/information.h \
    ../../../../view/horscreen/lifestyle.h \
    ../../../../view/horscreen/menumodule.h \
    ../../../../view/horscreen/nonetworkwidget.h \
    ../../../../view/horscreen/onedayweather.h \
    ../../../../view/horscreen/onehourweather.h \
    ../../../../view/horscreen/perhour.h \
    ../../../../view/horscreen/promptwidget.h \
    ../../../../view/horscreen/splitline.h \
    ../../../../view/horscreen/sport.h \
    ../../../../view/horscreen/title.h \
    ../../../../view/horscreen/ultravioletrays.h \
    ../../../../view/horscreen/washcar.h \
    ../../../../view/horscreen/weekweather.h \
    ../../../../view/horscreen/changecitywidget.h \
    ../../../../view/horscreen/picturetowhite.h \
    ../../../../view/search/searchview.h \
    ../../../../view/search/collectcity.h \
    ../../../../view/search/collectitemdelegate.h \
    ../../../../view/search/hotcity.h \
    ../../../../view/search/hotcitybtn.h \
    ../../../../view/search/searchcitylist.h \
    ../../../../view/search/searchdelegate.h \
    ../../../../view/search/titlesearchcityview.h \
    ../../../../view/search/titlesearchlistdelegate.h \
    ../../../../view/search/bottomlistview.h \
    ../../../../view/search/bottomlistdelegate.h \
    ../../../../view/verscreen/verscreen.h

# 包含路径
INCLUDEPATH += \
    . \
    ../../../../ \
    ../../../../controller \
    ../../../../controller/core \
    ../../../../model \
    ../../../../global \
    ../../../../view \
    ../../../../view/horscreen \
    ../../../../view/verscreen \
    ../../../../kabase \
    ../../../../kabase/Qt \
    ../../../../tests/view/search/searchview \
    /usr/include/kabase/ \
    /usr/include/kysdk/applications/kabase/kylin_system/ \
    /usr/include/kabase/kylin_system/ \
    /usr/include/kysdk/applications/kabase/ \
    /usr/include/kysdk/qtwidgets/ \
    /usr/include/x86_64-linux-gnu/qt5/QGSettings

# pkgconfig
PKGCONFIG += kysdk-qtwidgets kysdk-log kysdk-waylandhelper kysdk-kabase gsettings-qt kysdk-datacollect kysdk-ukuiwindowhelper

# 库路径和链接
LIBS += -L/usr/lib/kysdk/kysdk-base -lkylog -lkyconf

# 定义测试数据目录
DEFINES += SRCDIR=\"$$PWD/\"

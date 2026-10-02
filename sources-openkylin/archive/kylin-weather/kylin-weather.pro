QT       += core gui dbus
QT       += network x11extras KWindowSystem
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = kylin-weather
TEMPLATE = app

contains(CONFIG, KAIMING){
   message("kaiming")
   qm_files.path = /opt/apps/top.openkylin.kylin-weather/files/share/kylin-weather/translations
   settings.path = /opt/apps/top.openkylin.kylin-weather/files/share/glib-2.0/schemas/
   dbusService.path = /opt/apps/top.openkylin.kylin-weather/files/share/dbus-1/services
} else {
   message("not kaiming")
   qm_files.path = /usr/share/kylin-weather/translations/
   settings.path = /usr/share/glib-2.0/schemas/
   dbusService.path = /usr/share/dbus-1/services
   DEFINES += NOTKAIMING
}

# about widget version
VERSION = 3.2.0.3-ok16.11k1.2
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

CONFIG += link_pkgconfig c++11
QMAKE_CXXFLAGS += -Wall -g -O0

PKGCONFIG += kysdk-qtwidgets kysdk-log kysdk-waylandhelper kysdk-kabase gsettings-qt kysdk-datacollect kysdk-ukuiwindowhelper

LIBS += -L/usr/lib/kysdk/kysdk-base  -lkylog -lkyconf
#sdk不一样的版本路径有更改
INCLUDEPATH += /usr/include/kabase/
INCLUDEPATH += /usr/include/kysdk/applications/kabase/kylin_system/
INCLUDEPATH += /usr/include/kabase/kylin_system/
INCLUDEPATH += /usr/include/kysdk/applications/kabase/

INCLUDEPATH += kabase/
INCLUDEPATH += kabase/Qt

!system($$PWD/translations/generate_translations_pm.sh): error("Failed to generate pm")

TRANSLATIONS += \
    translations/kylin-weather_zh_CN.ts \
    translations/kylin-weather_kk.ts \
    translations/kylin-weather_ky.ts \
    translations/kylin-weather_ug.ts \
    translations/kylin-weather_bo_CN.ts \
    translations/kylin-weather_zh_HK.ts \
    translations/kylin-weather_de.ts \
    translations/kylin-weather_es.ts \
    translations/kylin-weather_fr.ts \
    translations/kylin-weather_mn.ts \
    translations/kylin-weather_vi.ts \
    translations/kylin-weather_th.ts \
    translations/kylin-weather_ms.ts


#settings.files = data/org.kylin.weather.gschema.xml
qm_files.files = translations/*.qm
#qm_files.path = /usr/share/kylin-weather/translations/
settings.files += \
    $$PWD/data/org.kylin.weather.gschema.xml \
    $$PWD/data/org.ukui.log4qt.kylin-weather.gschema.xml
#settings.path = /usr/share/glib-2.0/schemas/

target.source += $$TARGET
target.path = /usr/bin

icons.files += res/kylin-weather.png
icons.path = /usr/share/pixmaps/

appdesktop.files += kylin-weather.desktop
appdesktop.path = /usr/share/applications/

startdesktop.files += data/autostart/kylin-weather.desktop
startdesktop.path = /etc/xdg/autostart/

backgroundimages.files += res/background/*.png
backgroundimages.path = /usr/share/kylin-weather/background/

guide.files = data/guide/kylin-weather/
guide.path = /usr/share/kylin-user-guide/data/guide/

#json.files += kylin-weather.json
#json.path = /etc/ukui/ukui-notification/default-app/

dbusService.files = data/org.kylin.weather.service

INSTALLS += appdesktop startdesktop icons target qm_files guide settings json dbusService

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    controller/core/core.cpp \
    controller/core/coreinteraction.cpp \
    controller/core/errorreact.cpp \
    controller/core/viewcontrol.cpp \
    global/corevar.cpp \
    global/viewvar.cpp \
    main.cpp \
    model/dataparser.cpp \
    model/datarequest.cpp \
    model/dbus.cpp \
    model/iplocation.cpp \
    model/network.cpp \
    model/searchmarch.cpp \
    view/horscreen/air.cpp \
    view/horscreen/allowlocation.cpp \
    view/horscreen/city.cpp \
    view/horscreen/clothe.cpp \
    view/horscreen/empty.cpp \
    view/horscreen/flu.cpp \
    view/horscreen/horscreen.cpp \
    view/horscreen/information.cpp \
    view/horscreen/lifestyle.cpp \
    view/horscreen/menumodule.cpp \
    view/horscreen/nonetworkwidget.cpp \
    view/horscreen/onedayweather.cpp \
    view/horscreen/onehourweather.cpp \
    view/horscreen/perhour.cpp \
    view/horscreen/promptwidget.cpp \
    view/horscreen/splitline.cpp \
    view/horscreen/sport.cpp \
    view/horscreen/title.cpp \
    view/horscreen/ultravioletrays.cpp \
    view/horscreen/washcar.cpp \
    view/horscreen/weekweather.cpp \
    view/search/collectcity.cpp \
    view/search/collectitemdelegate.cpp \
    view/search/hotcity.cpp \
    view/search/hotcitybtn.cpp \
    view/search/searchcitylist.cpp \
    view/search/searchdelegate.cpp \
    view/search/searchview.cpp \
    view/verscreen/verscreen.cpp \
    view/horscreen/changecitywidget.cpp \
    view/horscreen/picturetowhite.cpp \
    view/search/titlesearchcityview.cpp \
    view/search/titlesearchlistdelegate.cpp \
    view/search/bottomlistview.cpp \
    view/search/bottomlistdelegate.cpp \
    model/titlesearchmodel.cpp \
    global/rotatechangeinfo.cpp \
    

HEADERS += \
    controller/Interaction.h \
    controller/core/core.h \
    controller/core/coreinteraction.h \
    controller/core/errorreact.h \
    controller/core/viewcontrol.h \
    global/corevar.h \
    global/viewvar.h \
    kabase/buriedpoint.hpp \
    kabase/Qt/kabaselog.hpp \
    kabase/utils.hpp \
    kabase/Qt/windowmanage.hpp \
    model/dataparser.h \
    model/datarequest.h \
    model/dbus.h \
    model/iplocation.h \
    model/network.h \
    model/searchmarch.h \
    view/horscreen/air.h \
    view/horscreen/allowlocation.h \
    view/horscreen/city.h \
    view/horscreen/clothe.h \
    view/horscreen/empty.h \
    view/horscreen/flu.h \
    view/horscreen/horscreen.h \
    view/horscreen/information.h \
    view/horscreen/lifestyle.h \
    view/horscreen/menumodule.h \
    view/horscreen/nonetworkwidget.h \
    view/horscreen/onedayweather.h \
    view/horscreen/onehourweather.h \
    view/horscreen/perhour.h \
    view/horscreen/promptwidget.h \
    view/horscreen/splitline.h \
    view/horscreen/sport.h \
    view/horscreen/title.h \
    view/horscreen/ultravioletrays.h \
    view/horscreen/washcar.h \
    view/horscreen/weekweather.h \
    view/search/collectcity.h \
    view/search/collectitemdelegate.h \
    view/search/hotcity.h \
    view/search/hotcitybtn.h \
    view/search/searchcitylist.h \
    view/search/searchdelegate.h \
    view/search/searchview.h \
    view/verscreen/verscreen.h \
    view/horscreen/changecitywidget.h \
    view/horscreen/picturetowhite.h \
    view/search/titlesearchcityview.h \
    view/search/titlesearchlistdelegate.h \
    view/search/bottomlistview.h \
    view/search/bottomlistdelegate.h \
    model/titlesearchmodel.h \
    global/rotatechangeinfo.h \

DISTFILES += \
    data/org.kylin.weather.gschema.xml \
    data/org.ukui.log4qt.kylin-weather.gschema.xml \
    translations/kylin-weather_bo_CN.ts \
    translations/kylin-weather_zh_CN.ts

RESOURCES += \
    data.qrc \
    res.qrc

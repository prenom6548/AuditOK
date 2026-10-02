QT       += core gui dbus network KWindowSystem sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 link_pkgconfig

# 项目名
TARGET = kylin-background-upgrade
TEMPLATE = app

LIBS +=-lukui-log4qt -lpython2.7

# 执行文件装载
target.path = /usr/bin
target.source += $$TARGET

SOURCES += \
    ./src/core.cpp \
    ./src/main.cpp \
    ./src/traydbus.cpp \
    ./src/trayicon.cpp \
    ./src/updatehandle.cpp \
    ./src/updatewidget.cpp

HEADERS += \
    ./src/core.h \
    ./src/traydbus.h \
    ./src/trayicon.h \
    ./src/updatehandle.h \
    ./src/updatewidget.h

# service文件装载  manul
service_manul.files = ./data/kylin-background-upgrade-manul.service
service_manul.path = /usr/lib/systemd/user/

# timer文件装载
timer_manul.files = ./data/kylin-background-upgrade-manul.timer
timer_manul.path  = /usr/lib/systemd/user/


# desktop文件装载
desktop.files = ./data/kylin-background-upgrade.desktop
desktop.path = /usr/share/applications/

conf.files = ./data/kylin-background-upgrade-template.conf
conf.path = /var/lib/kylin-background-upgrade/

# 日志配置文件
schemes.files += data/org.ukui.log4qt.kylin-background-upgrade.gschema.xml
schemes.path = /usr/share/glib-2.0/schemas/


TRANSLATIONS   += translations/kylin-background-upgrade_zh_CN.ts
                  translations/kylin-background-upgrade_bo_CN.ts
                  translation.path = /usr/share/kylin-background-upgrade
                  translation.files += translations/kylin-background-upgrade_zh_CN.qm
                  translation.files += translations/kylin-background-upgrade_bo_CN.qm


INSTALLS += target desktop conf translation schemes service_manul timer_manul

PKGCONFIG  += gsettings-qt

FORMS += \
    ./src/updatewidget.ui

RESOURCES += \
    ./data/Resources.qrc

DISTFILES += \
    ./data/update.png

DISTFILES += \
    checkUpdate.py

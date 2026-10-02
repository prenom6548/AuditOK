#-------------------------------------------------
#
# Project created by QtCreator 2019-12-21T09:41:12
#
#-------------------------------------------------

QT       += core gui network x11extras svg xml dbus KWindowSystem concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets quickwidgets

TARGET = ukui-control-center
DESTDIR = ..
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include(../env.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/Label/label.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/FlowLayout/flowlayout.pri)
include($$PROJECT_ROOTDIR/libukcc/interface.pri)

DEFINES += PLUGIN_INSTALL_DIRS='\\"$${PLUGIN_INSTALL_DIRS}\\"'
DEFINES += PLUGIN_INSTALL_UPDATE='\\"$${PLUGIN_INSTALL_UPDATE}\\"'

LIBS += -L$$[QT_INSTALL_LIBS] -lX11 -lgsettings-qt -ldconf -lukui-log4qt

DEFINES+= KYDEBUG

##加载gio库和gio-unix库，用于处理desktop文件
CONFIG        += link_pkgconfig \
                 C++11
PKGCONFIG     += gio-2.0 \
                 gio-unix-2.0 \
                 gsettings-qt \
                 kysdk-waylandhelper \
                 kysdk-datacollect \
                 kysdk-sysinfo \
                 kysdk-qtwidgets \
                 kysdk-waylandhelper \
                 kysdk-ukuiwindowhelper \
                 kysdk-package \

DBUS_ADAPTORS += $$PROJECT_ROOTDIR/data/org.ukui.ukcc.search.xml
DBUS_INTERFACES += $$PROJECT_ROOTDIR/data/org.ukui.ukcc.search.xml

desktop.files += ukui-control-center.desktop
desktop.path = /usr/share/applications

schemes.files += $$PROJECT_ROOTDIR/data/*.xml
schemes.path = /usr/share/glib-2.0/schemas/

face.files += $$PROJECT_ROOTDIR/data/faces/
face.path = /usr/share/ukui/

moZH.files += $$PROJECT_ROOTDIR/data/zh_CN/installer-timezones.mo
moZH.path = /usr/share/locale/zh_CN/LC_MESSAGES/

moBO.files += $$PROJECT_ROOTDIR/data/bo_CN/installer-timezones.mo
moBO.path = /usr/share/locale/bo_CN/LC_MESSAGES/

moHK.files += $$PROJECT_ROOTDIR/data/zh_HK/installer-timezones.mo
moHK.path = /usr/share/locale/zh_HK/LC_MESSAGES/

guideCN.files += $$PROJECT_ROOTDIR/data/zh_CN/
guideCN.path += /usr/share/kylin-user-guide/data/guide/ukui-control-center/

guideEN.files += $$PROJECT_ROOTDIR/data/en_US/
guideEN.path += /usr/share/kylin-user-guide/data/guide/ukui-control-center/

imageformats.files += $$PROJECT_ROOTDIR/data/ukui-control-center.png
imageformats.path = /usr/share/kylin-user-guide/data/guide/ukui-control-center/

target.source += $$TARGET
target.path = /usr/bin

search.files += $$PROJECT_ROOTDIR/data/org.ukui.ukcc.search.service
search.path = /usr/share/dbus-1/services/

modules.files += $$PROJECT_ROOTDIR/data/ukui-control-center-config.json
modules.path += /usr/share/ukui-control-center/data/

ukccYaml.files +=  $$PROJECT_ROOTDIR/data/*.yaml
ukccYaml.path += /etc/kylin-config/basic/

kylinbotSkill.files += $$PROJECT_ROOTDIR/data/ukui-control-center
kylinbotSkill.path = /opt/kylinbot/skills

INSTALLS +=  \
            target  \
            desktop \
            schemes \
            face    \
            moZH    \
            moBO    \
            moHK    \
            guideCN \
            imageformats \
            guideEN  \
            modules \
            search   \
            ukccYaml \
            kylinbotSkill \

INCLUDEPATH += qtsingleapplication \
               /usr/include/dconf

DEPENDPATH += qtsingleapplication

SOURCES += \
    customstyle.cpp \
    leftsidebarwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    homepagewidget.cpp \
    modulepagewidget.cpp \
    mstackwidget.cpp \
    pinyin.cpp \
    prescene.cpp \
    searchwidget.cpp \
    titlewidget.cpp \
    type.cpp \
    ukccsearchserver.cpp \
    qtsingleapplication/qtsingleapplication.cpp \
    qtsingleapplication/qtlocalpeer.cpp \
    utils/functionselect.cpp \
    utils/keyvalueconverter.cpp \
    utils/modulefactory.cpp \
    utils/xatom-helper.cpp \
    utils/scrollarea.cpp \

HEADERS += \
    customstyle.h \
    leftsidebarwidget.h \
    mainwindow.h \
    interface.h \
    homepagewidget.h \
    modulepagewidget.h \
    mstackwidget.h \
    pinyin.h \
    prescene.h \
    searchwidget.h \
    titlewidget.h \
    type.h \
    ukccsearchserver.h \
    qtsingleapplication/qtsingleapplication_copy.h \
    qtsingleapplication/qtsingleapplication.h \
    qtsingleapplication/qtlocalpeer.h \
    utils/functionselect.h \
    utils/keyvalueconverter.h \
    utils/modulefactory.h \
    utils/xatom-helper.h \
    utils/scrollarea.h \

FORMS += \
    mainwindow.ui \
    homepagewidget.ui \
    modulepagewidget.ui

RESOURCES += \
    res/resfile.qrc

DISTFILES += \
    ../data/installer-timezones.mo \
    ../data/org.ukui.control-center.panel.plugins.gschema.xml \
    ../data/org.ukui.control-center.personalise.gschema.xml \
    ../data/org.ukui.control-center.wifi.switch.gschema.xml \
    ../data/org.ukui.control-center.vino.xml \
    ../data/ukui-control-center-config.json \
    res/search.xml

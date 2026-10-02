#-------------------------------------------------
#
# ukui-system-monitor
#
#-------------------------------------------------

QT += core  gui dbus network KWindowSystem charts xml concurrent

isEqual(QT_MAJOR_VERSION, 5) {
    QT += widgets gui svg
}

include(QtSingleApplication/qtsingleapplication.pri)
include(procnet-monitor/procnet-monitor.pri)

TARGET = ukui-system-monitor
TEMPLATE = app
DESTDIR = ..
DEFINES += QT_MESSAGELOGCONTEXT
DEFINES += ENBALE_TRAY

LIBS += -L/usr/lib/  -lpcap -lm -lpthread -lukui-log4qt

CONFIG += link_pkgconfig \
                 c++11

PKGCONFIG += kysdk-waylandhelper \
             libsystemd \
             gsettings-qt \
             kysdk-utils \
             kysdk-qtwidgets \
             kysdk-diagnostics \
             kysdk-sysinfo \
             kysdk-proc \
             kysdk-hardware \
             kysdk-net \
             kysdk-realtime \
             kysdk-disk \
             kysdk-package \
             kysdk-ukuiwindowhelper

# Automating generation .qm files from .ts files
CONFIG(release, debug|release) {
    !system($$PWD/translate_generation.sh): error("Failed to generate translation")
}

ICON.files = img/ukui-system-monitor.png
ICON.path = /usr/share/icons/hicolor/

desktop.files += ../ukui-system-monitor.desktop
desktop.path = /usr/share/applications/

config.files += ../ukui-system-monitor.conf
config.path = /usr/share/ukui/

target.source += $$TARGET
target.path = /usr/bin

schemes.files += ../data/*.xml
schemes.path = /usr/share/glib-2.0/schemas/

json.files += ../ukui-system-monitor.json
json.path = /etc/ukui/ukui-notification/default-app/


INSTALLS += target \
            desktop \
            schemes \
            ICON \
            config \
            json

unix {
    UI_DIR = .ui
    MOC_DIR = .moc
    OBJECTS_DIR = .obj
}

HEADERS += \
    controls/kchartview.h \
    controls/kprogressbar.h \
    dialog/customservicenamedlg.h \
    dialog/onclosedialog.h \
    dialog/settingdialog.h \
    dialog/uslider.h \
    filesystem/datacdrom.h \
    filesystem/filesysteminfoitem.h \
    filesystem/filesystemwidget.h \
    kleftwidget.h \
    krightwidget.h \
    macro.h \
    mainwindow.h \
    prescene.h \
    process/processwidget.h \
    process/processwndinfo.h \
    service/kerror.h \
    service/dbuspropertiesinterface.h \
    service/servicecommon.h \
    service/servicedataworker.h \
    service/servicefileinfo.h \
    service/serviceinfo.h \
    service/servicemanager.h \
    service/servicesortfilterproxymodel.h \
    service/servicetablemodel.h \
    service/servicetableview.h \
    service/servicewidget.h \
    service/loadingwidget.h \
    service/systemd1managerinterface.h \
    service/systemd1serviceinterface.h \
    sysresource/basedetailviewwidget.h \
    sysresource/chartitemwidget.h \
    sysresource/chartviewscrollarea.h \
    sysresource/commoninfo.h \
    sysresource/cpudetailsmodel.h \
    sysresource/cpudetailswidget.h \
    sysresource/cpuhistorywidget.h \
    sysresource/detaillisttable.h \
    sysresource/diskdetailsmodel.h \
    sysresource/diskdetailswidget.h \
    sysresource/diskhistorywidget.h \
    sysresource/memdetailsmodel.h \
    sysresource/memdetailswidget.h \
    sysresource/memhistorywidget.h \
    sysresource/netdetailsmodel.h \
    sysresource/netdetailswidget.h \
    sysresource/nethistorywidget.h \
    trayicon.h \
    util.h \
    desktopfileinfo.h \
    controls/mydialog.h \
    controls/myactiongroup.h \
    controls/myactiongroupitem.h \
    controls/myaction.h \
    controls/kitemdelegate.h \
    controls/kheaderview.h \
    controls/ktableview.h \
    filesystem/filesystemworker.h \
    filesystem/filesystemdata.h \
    filesystem/filesystemwatcher.h \
    dialog/renicedialog.h \
    dialog/procpropertiesdlg.h \
    process/processtableview.h \
    process/process_data.h \
    process/process_list.h \
    process/process_monitor.h \
    process/process_network.h \
    process/processtablemodel.h \
    process/processsortfilterproxymodel.h \
    style/usmproxystyle.h
    

SOURCES += \
    controls/kchartview.cpp \
    controls/kprogressbar.cpp \
    dialog/customservicenamedlg.cpp \
    dialog/onclosedialog.cpp \
    dialog/settingdialog.cpp \
    dialog/uslider.cpp \
    filesystem/datacdrom.cpp \
    filesystem/filesysteminfoitem.cpp \
    filesystem/filesystemwidget.cpp \
    kleftwidget.cpp \
    krightwidget.cpp \
    mainwindow.cpp \
    prescene.cpp \
    process/processwidget.cpp \
    process/processwndinfo.cpp \
    service/kerror.cpp \
    service/dbuspropertiesinterface.cpp \
    service/servicedataworker.cpp \
    service/servicefileinfo.cpp \
    service/serviceinfo.cpp \
    service/servicemanager.cpp \
    service/servicesortfilterproxymodel.cpp \
    service/servicetablemodel.cpp \
    service/servicetableview.cpp \
    service/servicewidget.cpp \
    service/loadingwidget.cpp \
    service/systemd1managerinterface.cpp \
    service/systemd1serviceinterface.cpp \
    sysresource/basedetailviewwidget.cpp \
    sysresource/chartitemwidget.cpp \
    sysresource/chartviewscrollarea.cpp \
    sysresource/commoninfo.cpp \
    sysresource/cpudetailsmodel.cpp \
    sysresource/cpudetailswidget.cpp \
    sysresource/cpuhistorywidget.cpp \
    sysresource/detaillisttable.cpp \
    sysresource/diskdetailsmodel.cpp \
    sysresource/diskdetailswidget.cpp \
    sysresource/diskhistorywidget.cpp \
    sysresource/memdetailsmodel.cpp \
    sysresource/memdetailswidget.cpp \
    sysresource/memhistorywidget.cpp \
    sysresource/netdetailsmodel.cpp \
    sysresource/netdetailswidget.cpp \
    sysresource/nethistorywidget.cpp \
    trayicon.cpp \
    main.cpp \
    util.cpp \
    desktopfileinfo.cpp \
    controls/mydialog.cpp \
    controls/myactiongroup.cpp \
    controls/myactiongroupitem.cpp \
    controls/myaction.cpp \
    controls/kitemdelegate.cpp \
    controls/kheaderview.cpp \
    controls/ktableview.cpp \
    filesystem/filesystemworker.cpp \
    filesystem/filesystemdata.cpp \
    filesystem/filesystemwatcher.cpp \
    dialog/renicedialog.cpp \
    dialog/procpropertiesdlg.cpp \
    process/processtableview.cpp \
    process/process_list.cpp \
    process/process_monitor.cpp \
    process/process_network.cpp \
    process/processtablemodel.cpp \
    process/processsortfilterproxymodel.cpp \
    style/usmproxystyle.cpp

RESOURCES += \
    res.qrc

TRANSLATIONS += \
    translation/ukui-system-monitor_bo_CN.ts \
    translation/ukui-system-monitor_zh_CN.ts \
    translation/ukui-system-monitor_en_US.ts \
    translation/ukui-system-monitor_mn.ts   \
    translation/ukui-system-monitor_zh_HK.ts \
    translation/ukui-system-monitor_kk.ts \    # 哈萨克
    translation/ukui-system-monitor_ug.ts \    # 维吾尔
    translation/ukui-system-monitor_ky.ts \    # 柯尔克孜
    translation/ukui-system-monitor_de.ts \    # 德语
    translation/ukui-system-monitor_es.ts \    # 西班牙
    translation/ukui-system-monitor_fr.ts \    # 法语
    translation/ukui-system-monitor_vi.ts \    # 京语
    translation/ukui-system-monitor_ms.ts \    # 马来语
    translation/ukui-system-monitor_th.ts \    # 泰语

DISTFILES += \
    img/nosearchresult_black.svg \
    img/nosearchresult_white.svg

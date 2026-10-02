#include <QCoreApplication>
#include <QTranslator>
#include <QDebug>
#include "launch_printer.h"
#include <QLibraryInfo>
#include <QDir>
#include "device_monitor.h"
#include "launch_printer.h"
#include "common.h"
#include "event_notify_monitor.h"
#include "device_map.h"
#include "pop_window_manager.h"
#include "find_and_match_ppds.h"
#include "printer_backend.h"


void addTrans()
{
//加载中文
    QString qtTranslationsPath;
    qtTranslationsPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);// /usr/share/qt5/translations
    QString kylinPrinterTranslationsPath;
#ifndef TEST_DEBUG
    if (QDir("/usr/share/kylin-os-manager/daemons/sessionservice/printerbackend/translations").exists()) {
        kylinPrinterTranslationsPath = "/usr/share/kylin-os-manager/daemons/sessionservice/printerbackend/translations";
    }
    else {
        kylinPrinterTranslationsPath = QCoreApplication::applicationDirPath() + "/.qm";
    }

#else
    kylinPrinterTranslationsPath = QString(PROJECT_SOURCE_DIR) + "/translations";
#endif
    qDebug () << kylinPrinterTranslationsPath;
    QString locale = QLocale::system().name();
    static QTranslator trans_global, trans_menu, trans_dict, trans_sdk;

    if(!trans_global.load("kylin-printer_" + locale + ".qm", kylinPrinterTranslationsPath))
        qDebug() << "Load translations file" <<QLocale() << "failed!";
    else
        QCoreApplication::installTranslator(&trans_global);

    if(!trans_menu.load("qt_" + locale + ".qm", qtTranslationsPath))
        qDebug() << "Load translations file" <<QLocale() << "failed!";
    else
        QCoreApplication::installTranslator(&trans_menu);

    if(!trans_sdk.load(":/translations/gui_" + locale + ".qm"))
        qDebug() << "Load translations file" <<QLocale() << "failed!";
    else
        QCoreApplication::installTranslator(&trans_sdk);
}

bool checkKylinPrinter()
{
    // kylin-printer 不存在返回true
    QString version = getPackageVersion("kylin-printer");

    qDebug() << "kylin-printer version is " << version;

    if (version.isEmpty()) {
        return true;
    }
    // version < 1.3.1.0-0k4.0
    // dpkg --compare-versions verison lt 1.3.1.0-0k4.0 && echo true
    QString cmd = QString("dpkg --compare-versions %1 lt 1.3.1.0-0k4.0 && echo true").arg(version);
    QString status = ::getRetFromCommand(cmd.split(" "));
    if (status == "true") {
        // verison >= 1.1.0kord-0k12~rc32
        // dpkg --compare-versions verison ge 1.1.0kord-0k12~rc32 && echo true
        cmd = QString("dpkg --compare-versions %1 ge 1.1.0kord-0k12~rc32 && echo true").arg(version);
        QString status = ::getRetFromCommand(cmd.split(" "));
        if (status == "true") {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

void doPrinterWork()
{
    if (!checkKylinPrinter()) {
        return;
    }
    addTrans();

    g_findAndMatchWorkThread->start();
    g_findAndMatchWorkThread->updatePPDsMap();

    LaunchPrinter::getInstance();
    PopWindowManager::getInstance();

    DeviceMonitorBackend *p = DeviceMonitorBackend::getInstance();
    QObject::connect (p,  &DeviceMonitorBackend::usbDeviceAdd,
                    LaunchPrinter::getInstance(), &LaunchPrinter::add);

    QObject::connect (p,  &DeviceMonitorBackend::usbDeviceRemove,
                    LaunchPrinter::getInstance(), &LaunchPrinter::remove);
    emit DeviceMonitorBackend::getInstance()->start();

    DeviceMap::getInstance();

    EventNotifyMonitor *notify = EventNotifyMonitor::getInstance();
    emit notify->start();
}
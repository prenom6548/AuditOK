#include <QCoreApplication>
#include <QApplication>
#include <QObject>
#include <QDebug>
#include <QThread>
#include <QTranslator>
#include <QLocale>
#include <QStandardPaths>
#include <QLibraryInfo>
#include <QMutex>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QObject>

#include <sys/inotify.h>
#include <fcntl.h>
#include <syslog.h>

#include <ukui-log4qt.h>

#include "device_monitor.h"
#include "launch_printer.h"
#include "common.h"
#include "event_notify_monitor.h"
#include "test.h"
#include "ukui_printer.h"
#include "pop_window_manager.h"
int main(int argc, char **argv) {

    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication app(argc, argv);

#ifndef TEST_DEBUG
    initUkuiLog4qt("kylin-printer-applet");
#else
    qInstallMessageHandler(myMessageOutput);
#endif
    QString vendor = "Brother";
    QString model = "HL-B2050DN";
    QString accuratelyMatchPPD;
    QStringList fuzzyMatchPPDlis;
    PopWindowManager::getInstance()->findPPD(vendor, model,
                 accuratelyMatchPPD,
                 fuzzyMatchPPDlis);
    EventNotifyMonitor *notify = EventNotifyMonitor::getInstance();
//    Test *test = new Test();
//    QObject::connect (notify,  &EventNotifyMonitor::getNotify,
//                      test,    &Test::show);
    emit notify->start();

    QString testName = "test";
    QString d = UkuiPrinterManager::getInstance().getDefaultPrinter();
    qDebug() << d;
    UkuiPrinterManager::getInstance().duplicatePrinter(testName, d);
    UkuiPrinterManager::getInstance().printTestPage(testName);
    UkuiPrinterManager::getInstance().removePrinter(testName);
    return app.exec();
}

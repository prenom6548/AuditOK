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

#include "event_notify_monitor.h"
#include "device_map.h"
#include "ukui_printer.h"
#include "common.h"


// 插拔打印机识别并弹窗功能测试用例：
// 1.cups删除全部打印机 
// 2.清空gsetting:
//     gsettings reset org.kylin-printer.settings printers
// 3.插入打印机，提示打印机安装成功；
// 4.拔出打印机，提示打印机已经拔出；
// 5.再次插入打印机，提示打印机已经插入；
// 6.重启电脑，插拔打印机弹窗显示正常
// 7. 删除打印机后，拔出打印机无弹窗，再次插入打印机后，打印机弹出正在安装界面

int main(int argc, char **argv) {

    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication app(argc, argv);

#ifndef TEST_DEBUG
    initUkuiLog4qt("kylin-printer-applet");
#else
    qInstallMessageHandler(myMessageOutput);
#endif

    EventNotifyMonitor *notify = EventNotifyMonitor::getInstance();
    emit notify->start();
    DeviceMap::getInstance();

    QString testName = "test";
    QString d = UkuiPrinterManager::getInstance().getDefaultPrinter();
    qDebug() << d;
    UkuiPrinterManager::getInstance().duplicatePrinter(testName, d);
    // UkuiPrinterManager::getInstance().printTestPage(testName);
    UkuiPrinterManager::getInstance().removePrinter(testName);

    return app.exec();
}

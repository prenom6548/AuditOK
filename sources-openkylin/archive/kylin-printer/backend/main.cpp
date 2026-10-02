/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
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

#include <singleapplication.h>
#include "device_monitor.h"
#include "launch_printer.h"
#include "common.h"
#include "log.h"
#include "tray_win.h"
#include "event_notify_monitor.h"
#include "device_map.h"
#include "pop_window_manager.h"
#include "find_and_match_ppds.h"
#include "printer_check_helper.h"
#include "command_helper.h"


QString readEnvFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开文件:" << filePath;
        return QString();
    }

    QTextStream in(&file);
    QString firstLine = in.readLine(); // 只读取第一行
    file.close();
    return firstLine.trimmed(); 
}

void addTrans()
{
//加载中文
    QString qtTranslationsPath;
    qtTranslationsPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);// /usr/share/qt5/translations
    QString kylinPrinterTranslationsPath;
#ifndef TEST_DEBUG
    if (QDir("/usr/share/kylin-printer/translations").exists()) {
        kylinPrinterTranslationsPath = "/usr/share/kylin-printer/translations";
    }
    else {
        kylinPrinterTranslationsPath = QApplication::applicationDirPath() + "/.qm";
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
        QApplication::installTranslator(&trans_global);

    if(!trans_dict.load("kylin-printer_dict_" + locale + ".qm", kylinPrinterTranslationsPath))
        qDebug() << "Load translations file" <<QLocale() << "failed!";
    else
        QApplication::installTranslator(&trans_dict);

    if(!trans_menu.load("qt_" + locale + ".qm", qtTranslationsPath))
        qDebug() << "Load translations file" <<QLocale() << "failed!";
    else
        QApplication::installTranslator(&trans_menu);

    if(!trans_sdk.load(":/translations/gui_" + locale + ".qm"))
        qDebug() << "Load translations file" <<QLocale() << "failed!";
    else
        QApplication::installTranslator(&trans_sdk);
}

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    kdk::QtSingleApplication app(argc, argv);

    if (!processCheck("/usr/sbin/cupsd")) {
        qCritical() << "Cannot find cups service!";
        return 0;
    }

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        qDebug()<<"I couldn't detect any system tray on this system now";
        return 1;
    }
    QApplication::setQuitOnLastWindowClosed(false);
#ifndef TEST_DEBUG
    qInstallMessageHandler(logOutput);
#else
    qInstallMessageHandler(myMessageOutput);
#endif

    QString commandArgs = QString("");
    if (argc > 1) {
        commandArgs = QString(argv[1]);
    } else {
        commandArgs = readEnvFile("/tmp/kylin-printer-applet-env");
    }
    if (app.isRunning()) {
        qDebug() << "is running";
        if (!commandArgs.isEmpty()) {
            app.sendMessage(commandArgs);
        }
        return 0;
    }

    addTrans();

    qRegisterMetaType<QString>("QString&");
    qRegisterMetaType<DeviceInformation>("DeviceInformation");
    qRegisterMetaType<DeviceInformation>("DeviceInformation&");

    g_findAndMatchWorkThread->start();
    g_findAndMatchWorkThread->updatePPDsMap();

    qDebug () << "LaunchPrinter run!";
    LaunchPrinter::getInstance();
    PopWindowManager::getInstance();

    DeviceMonitorBackend *p = DeviceMonitorBackend::getInstance();
    QObject::connect (p,  &DeviceMonitorBackend::usbDeviceAdd,
                      LaunchPrinter::getInstance(), &LaunchPrinter::add);

    QObject::connect (p,  &DeviceMonitorBackend::usbDeviceRemove,
                      LaunchPrinter::getInstance(), &LaunchPrinter::remove);
    emit DeviceMonitorBackend::getInstance()->start();
#if 0
    TrayWin tray;
    QObject::connect (notify,  &EventNotifyMonitor::getNotify,
                      &tray, &TrayWin::showNotify);
#endif
    DeviceMap::getInstance();
//    LaunchPrinter::getInstance()->findPrePrinter();
    EventNotifyMonitor *notify = EventNotifyMonitor::getInstance();
    emit notify->start();
    PrinterCheckHelper::getInstance();

    if (!commandArgs.isEmpty()) {
        QString command = QString(commandArgs);
        qDebug() << command;
        CommandHelper a(command);
    }

    // QObject::connect(&app, &kdk::QtSingleApplication::messageReceived, [&](QString message) {
    //     qDebug() << message;
    //     CommandHelper a(command);
    // });

#ifdef UNIT_TEST
    return 0;
#else
    return app.exec();
#endif
}

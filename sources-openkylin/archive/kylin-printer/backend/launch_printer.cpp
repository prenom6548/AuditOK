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
#include <QString>
#include <QStringList>
#include <QDataStream>
#include <QDBusConnection>

#include "launch_printer.h"
#include "device_monitor.h"
#include "common.h"
#include "device_map.h"
#include "global_variable.h"
#include "system_notification.h"
#include "ukui_printer.h"
#include "lpinfo_browser.h"
#include "pop_window_manager.h"
#include "upload_helper.h"

LaunchPrinter::LaunchPrinter(/* args */)
{

}

LaunchPrinter::~LaunchPrinter()
{

}

void LaunchPrinter::add(const QString &path)
{
    // "/devices/pci0000:00/0000:00:14.0/usb1/1-6"
    qDebug() << APP_PATH<<"add" << path;
    DeviceMap::getInstance()->checkPathAdd(path);
}

void LaunchPrinter::remove(const QString &path)
{
    qDebug() <<  APP_PATH<<"remove" << path;
    DeviceMap::getInstance()->checkPathRemove(path);
}

void LaunchPrinter::add_uri(const QString &path)
{
    qDebug() <<  APP_PATH<<"add_uri" << path;
}

void LaunchPrinter::findPrePrinter()
{
    QList<UkuiUsbPrinter> printers;
    UkuiPrinterManager::getInstance().enumUsbPrinters(printers);
    QMap<QString, DeviceInformation> devicelist = LpinfoBrowser::findDirectPrinter();

    // 如果之前安装过了， 就忽略
    for (const auto & device : devicelist) {
        if (device.deviceType == DeviceType::USB) {
            if (UkuiPrinterManager::getInstance().printerIsInstalled(device.uri, false, printers) == false)
                add_uri(device.uri);
        }
    }
}

LaunchPrinter *LaunchPrinter::getInstance()
{
    static LaunchPrinter *instance = nullptr;
    static QThread *thread;
    if (nullptr == instance) {
        instance = new LaunchPrinter();
        thread = new QThread();
        thread->setObjectName("LaunchPrinter");
        instance->moveToThread(thread);
        QDBusConnection::sessionBus().connect("org.freedesktop.Notifications",
                                              "/org/freedesktop/Notifications",
                                              "org.freedesktop.Notifications", "ActionInvoked", instance,
                                              SLOT(actionInvokeSlot(quint32, QString)));
        thread->start();
    }
    return instance;
}

void LaunchPrinter::actionInvokeSlot(quint32 id, QString actionkey)
{
    // kylin-printer-add-printer:syspath
    if (actionkey.contains("kylin-printer-add-printer:")) {
        qDebug() << actionkey;
        actionkey.remove("kylin-printer-add-printer:");
        PopWindowManager::getInstance()->installPrinter(actionkey);
    }
    // kylin-printer-print-test-page:deviceinfomation
    else if(actionkey.contains("kylin-printer-print-test-page:")) {
        qDebug() << actionkey;
        actionkey.remove("kylin-printer-print-test-page:");
        DeviceInformation info = unserializeDeviceInformation(actionkey);
        UkuiPrinterManager::getInstance().printTestPage(info.displayName);
        SystemNotification().sendMessage(tr("Is the test page printed successfully?"), tr("%1 print result confirmation").arg(info.displayName), -1,
                                         QStringList() << QString("kylin-printer-print-test-page-successfully:%1").arg(actionkey)
                                                       << tr("Yes")
                                                       << QString("kylin-printer-print-test-page-failed:%1").arg(actionkey)
                                                       << tr("No"));
    }
    // kylin-printer-print-test-page-successfully:deviceinfomation
    else if (actionkey.contains("kylin-printer-print-test-page-successfully:")) {
        qDebug() << actionkey;
        actionkey.remove("kylin-printer-print-test-page-successfully:");
        DeviceInformation info = unserializeDeviceInformation(actionkey);
        UploadHelper *u = new UploadHelper(info, 1);
        u->startWorker();
    }
    // kylin-printer-print-test-page-failed:deviceinfomation
    else if (actionkey.contains("kylin-printer-print-test-page-failed:")) {
        qDebug() << actionkey;
    }

}

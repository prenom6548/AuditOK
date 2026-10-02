/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#include <QCoreApplication>
#include <QThread>
#include <QDir>
#include <QDebug>
#include <libkylog.h>
#include "crash-collect.h"
#include "systemmonitor.h"
#include "kom_buriedpoint.h"
#include "obtainsysteminformation.h"

static void logOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    const char *message = localMsg.constData();
    const char *file = context.file ? context.file : "";
    const char *func = context.function ? context.function : "";

    switch (type) {
    case QtDebugMsg:
        klog_debug("%s (%s:%u,%s)\n", message, file, context.line, func);
        break;
    case QtInfoMsg:
        klog_info("%s (%s:%u,%s)\n", message, file, context.line, func);
        break;
    case QtWarningMsg:
        klog_warning("%s (%s:%u,%s)\n", message, file, context.line, func);
        break;
    case QtCriticalMsg:
        klog_err("%s (%s:%u,%s)\n", message, file, context.line, func);
        break;
    case QtFatalMsg:
        klog_emerg("%s (%s:%u,%s)\n", message, file, context.line, func);
        break;
    default:
        break;
    }
}

void countPeople()
{

    QFile file("/etc/machine-id");
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "打开 /etc/machine-id 失败 ！";
        return;
    }
    QString machine = file.readAll();
    file.close();
    if (machine.isEmpty()) {
        qDebug() << "读取 /etc/machine-id 失败 ！";
        return;
    }
    kom::BuriedPoint::uploadMessage(kom::BuriedPoint::BuriedPointPage::kMonitorService,
                                    kom::BuriedPoint::EventCode::kMonitorMachineId,
                                    {{"machineId", machine}});
}

int main(int argc, char *argv[])
{
#ifndef DEBUG_MODE
    qInstallMessageHandler(logOutput);
#endif

    QCoreApplication app(argc, argv);

    countPeople();             //统计使用率埋点
    ObtainSystemInformation(); //采集系统信息

    QDir dir("/usr/share/kylin-os-manager/komd-crash/");
    if (!dir.exists())
        if (!dir.mkpath("/usr/share/kylin-os-manager/komd-crash/"))
            qCritical() << "create komd-crash cache dir fail !";

    QThread *crashThread = new QThread(&app);
    crash::CrashCollect *crashCollect = new crash::CrashCollect;
    QObject::connect(crashThread, &QThread::started, crashCollect, &crash::CrashCollect::on_run);
    QObject::connect(crashThread, &QThread::finished, crashCollect, &crash::CrashCollect::deleteLater);
    crashCollect->moveToThread(crashThread);
    crashThread->start();

    systemmonitor::SystemMonitor::getInstance();

    countPeople();

    return app.exec();
}

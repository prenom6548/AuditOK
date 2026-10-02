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

#include <QDBusInterface>
#include <QDBusConnection>
#include <QDebug>
#include <QFile>
#include <QProcess>

#include "dbus.h"

Dbus::Dbus()
{
    QDBusConnection::systemBus().connect("", "/com/redhat/PrinterSpooler", "com.redhat.PrinterSpooler", "PrinterAdded",   this, SLOT(dbusPrinterAdded(QString)));
    QDBusConnection::systemBus().connect("", "/com/redhat/PrinterSpooler", "com.redhat.PrinterSpooler", "PrinterRemoved", this, SLOT(dbusPrinterRemoved(QString)));
}

Dbus::~Dbus()
{

}

bool Dbus::getxtty(QString &xtty, QString &xuser, QString &xuserid)
{
    xtty.clear();
    xuser.clear();
    xuserid.clear();


    // 1. 读取当前活动的 TTY
    QFile ttyFile("/sys/class/tty/tty0/active");
    if (!ttyFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open tty0/active";
        return false;
    }
    xtty = QString::fromUtf8(ttyFile.readLine()).trimmed();
    ttyFile.close();

    if (xtty.isEmpty()) {
        qDebug() << "No active tty found, exiting.";
        return false;
    }


    // 2. 获取当前 TTY 的用户
    QProcess whoProc;
    whoProc.start("who");
    whoProc.waitForFinished();
    QString whoOutput = QString::fromUtf8(whoProc.readAllStandardOutput());

    const QStringList lines = whoOutput.split("\n", QString::SkipEmptyParts);
    for (const QString &line : lines) {
        if (line.contains(xtty)) {
            xuser = line.section(' ', 0, 0);  // 第一个字段是用户名
            break;
        }
    }

    if (xuser.isEmpty()) {
        qDebug() << "No user found for tty" << xtty;
        return false;
    }


    // 3. 获取 UID
    QProcess idProc;
    idProc.start("id", {"-u", xuser});
    idProc.waitForFinished();
    xuserid = QString::fromUtf8(idProc.readAllStandardOutput()).trimmed();

    if (xuserid.isEmpty()) {
        qDebug() << "Failed to get UID for user" << xuser;
        return false;
    }
    return true;
}

void Dbus::dbusPrinterAdded(const QString &queueName)
{
    dbusPrinter("addprinter", queueName);
}

void Dbus::dbusPrinterRemoved(const QString &queueName)
{
    dbusPrinter("removeprinter", queueName);
}

void Dbus::dbusPrinter(const QString &action, const QString &queueName)
{
    QString program = "/usr/share/kylin-printer/kylin-printer-applet-start";
    QStringList arguments;
    arguments << action + "_" + queueName;

    QProcess::startDetached(program, arguments);
}
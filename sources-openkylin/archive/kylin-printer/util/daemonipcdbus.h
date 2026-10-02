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
#ifndef DAEMONIPCDBUS_H
#define DAEMONIPCDBUS_H

#include <QObject>
#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>
#include <unistd.h>
#include <sys/types.h>

//Command handler here...
class DaemonIpcDbus : public QObject
{
    Q_OBJECT
private:
    DaemonIpcDbus();
public:
    
    static DaemonIpcDbus *getInstance();
    void callRequest(QStringList argv);
signals:
    void show();
    void sendarg(QStringList arg);

public slots:
    int daemonIsNotRunning();
    void showGuide(QString appName);
    void request(QStringList command);
};

#endif // DAEMONIPCDBUS_H

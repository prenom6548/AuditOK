/* Copyright (C) 2020, KylinSoft Co., Ltd.
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
#ifndef DBUS_H
#define DBUS_H

#include <QObject>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusReply>

#include "global/variable.h"

class Dbus : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void processingCommand(QStringList);
    void processingFullScreen();

public:
    Dbus();
    void argumentsCommand(const QStringList &arguments);
    bool getConnectSeccess();

public Q_SLOTS:
    void getCmdFromOtherMe(const QStringList &cmd);
    void fullScreen();

private:
    bool m_connectSeccess = false; //注册DBus成功
};

#endif // DBUS_H

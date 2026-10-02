/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef LAN_H
#define LAN_H

#include <QObject>
#include <QDBusContext>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDBusReply>
#include <QDebug>
#include <QString>
#include <QStringList>

class Lan : public QObject, QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.Framework.Devices.LAN")
public:
    explicit Lan(QObject *parent = nullptr);
    virtual ~Lan();

public Q_SLOTS:
    /**
     * @brief IsEnable
     * @return
     */
    bool IsEnable();
    /**
     * @brief Enable
     * @param enable
     */
    void Enable(bool enable);

    bool SetDeviceState(QString device, bool enable);
    QStringList GetAllDevices(void);

Q_SIGNALS:
    void EnableChanged(bool);

private:
    QDBusPendingCallWatcher * __lan_dbus_call(const QString & method, const QList<QVariant> & args);

    template <typename...Args>
    QDBusMessage callNetworkMethod(const QString &method, Args &&...args);
};

#endif

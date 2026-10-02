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
#ifndef WIFI_H
#define WIFI_H

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

/*
    现主线系统不支持多网卡开关操作，因此仅能使用无线网络总开关；
    使用无阻塞方式调用和获取其他dbus服务提供的接口，防止线程长时间阻塞；
*/
class Wifi : public QObject, QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.Framework.Devices.WIFI")
public:
    explicit Wifi(QObject *parent = nullptr);
    virtual ~Wifi();

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
    QDBusPendingCallWatcher * __wifi_dbus_call(const QString & method, const QList<QVariant> & args);

    template <typename...Args>
    QDBusMessage callNetworkMethod(const QString &method, Args &&...args);
};

#endif // WIFI_H

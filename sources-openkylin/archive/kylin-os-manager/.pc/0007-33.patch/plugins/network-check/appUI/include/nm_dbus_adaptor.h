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

#ifndef NM_DBUS_ADAPTOR_H
#define NM_DBUS_ADAPTOR_H

#include <QObject>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusObjectPath>
#include <QDBusReply>
#include <QDBusObjectPath>
// #include "kylin-dbus-interface.h"

struct HWConfig
{
    HWConfig() {}
};
class NmDbusAdaptor : public QObject
{
    Q_OBJECT
public:
    explicit NmDbusAdaptor(QObject *parent = nullptr);
    ~NmDbusAdaptor();
    bool checkHWConfig(QStringList &configInfo);
    bool isWiredCableOn() const;
    bool isWirelessCardOn() const;
signals:
private:
    QList<QString> m_unvisibleDeviceNames; //需要屏蔽的网卡名称
    QList<QString> m_unvisibleDevicePaths; //需要屏蔽的网卡地址
    QList<QString> m_visibleDeviceNames;   //实际可见的网卡名称

    // KylinDBus *m_NmDBus = nullptr;
};

#endif // NETWORKMANAGERDBUS_H

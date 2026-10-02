/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef KYLINCONNECTRESOURCE_H
#define KYLINCONNECTRESOURCE_H

#include <QString>
#include "kylinnetworkresourcemanager.h"
#include "kylinnetresource.h"
#include "kylinconnectitem.h"
#include "kylinnetworkdeviceresource.h"
#include "kylinconnectsetting.h"
#include "kylinapconnectitem.h"

class KyWiredConnectResourse : public KyNetResource
{
    Q_OBJECT
public:
    explicit KyWiredConnectResourse(QObject *parent = nullptr);

public:
    void getWiredList(QMap<QString,QList<KyWiredItem>> &map);

    void getWiredConnectDetail(QString interface, QString uuid, KyDetailInfo &connectSetting);
private:
    void kyWirdNetItemListInit();
    KyWiredItem getConnectionItem(NetworkManager::Connection::Ptr connectPtr);

Q_SIGNALS:
    void wiredConnectionAdd(QString devName, QString connectUuid, QString connectName, QString connectPath);
    void wiredConnectionUpdate(QString devName, QString connectUuid, QString connectName, QString connectPath);
    void connectionRemove(QString path);

    void wiredDeviceAdd(QString deviceName);
    void wiredDeviceUpdate(QString oldDeviceName, QString newDeviceName);
    void deviceRemove(QString deviceName);

private:
    QMap<QString, QList<KyWiredItem>>   m_wiredNetworkMap;
};
#endif // KYLINCONNECTRESOURCE_H

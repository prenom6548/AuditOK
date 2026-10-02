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

#ifndef KYNETRESOURCE_H
#define KYNETRESOURCE_H

#include "kylinnetworkresourcemanager.h"
#include "kylinconnectsetting.h"
#include <NetworkManagerQt/Ipv4Setting>
#include <NetworkManagerQt/Ipv6Setting>

class KyDetailInfo
{
public:
    QString strSecType;
    QString strChan;
    QString strMac;
    QString strHz;
    QString strBandWidth;
    QString strDynamicIpv4;
    QString strDynamicIpv6;
    QString strDynamicIpv4Dns;
    bool    isAutoConnect = false;
};

class KyNetResource : public QObject
{
    Q_OBJECT
public:
    explicit KyNetResource(QObject *parent = nullptr);
    ~KyNetResource();

    void getConnectionSetting(QString connectUuid, KyConnectSetting &connectSetting);

protected:
    KyNetworkResourceManager *m_networkResourceInstance = nullptr;

    void getBaseInfo(QString devName,QString ssid, bool isWlan, bool isActive, KyDetailInfo &conInfo);
    void getDynamicIpInfo(QString uuid, KyDetailInfo &conInfo);

private:
    void getIpv4ConnectSetting(NetworkManager::Ipv4Setting::Ptr &ipv4Setting,
                                                    KyConnectSetting &connectSetting);
    void getIpv6ConnectSetting(NetworkManager::Ipv6Setting::Ptr &ipv6Setting,
                                                    KyConnectSetting &connectSetting);
};

#endif // KYNETRESOURCE_H

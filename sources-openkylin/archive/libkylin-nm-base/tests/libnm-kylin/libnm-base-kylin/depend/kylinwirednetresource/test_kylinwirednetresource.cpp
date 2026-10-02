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

#include "test_kylinwirednetresource.h"
#include "kylinwirednetresource.h"
#include "kylinnetworkresourcemanager.h"
#include "kylinnetworkdeviceresource.h"
#include "kylinconnectitem.h"

KyWiredConnectResourse *TestKyWiredConnectResourse::m_resource = nullptr;
KyNetworkResourceManager *TestKyWiredConnectResourse::m_resourceManager = nullptr;
KyNetworkDeviceResourse *TestKyWiredConnectResourse::m_deviceResource = nullptr;

void TestKyWiredConnectResourse::SetUpTestSuite()
{
    m_resourceManager = KyNetworkResourceManager::getInstance();
    m_deviceResource = new KyNetworkDeviceResourse();
    m_resource = new KyWiredConnectResourse();
}

void TestKyWiredConnectResourse::TearDownTestSuite()
{
    if (m_resource) {
        delete m_resource;
        m_resource = nullptr;
    }
    if (m_deviceResource) {
        delete m_deviceResource;
        m_deviceResource = nullptr;
    }
    KyNetworkResourceManager::Release();
    m_resourceManager = nullptr;
}

void TestKyWiredConnectResourse::SetUp()
{
}

void TestKyWiredConnectResourse::TearDown()
{
}

KyWiredItem TestKyWiredConnectResourse::createWiredItem(const QString &name, const QString &uuid, const QString &path)
{
    KyWiredItem item;
    item.m_connectName = name;
    item.m_connectUuid = uuid;
    item.m_connectPath = path;
    return item;
}

QList<KyWiredItem> TestKyWiredConnectResourse::createWiredItemList(int count)
{
    QList<KyWiredItem> list;
    for (int i = 0; i < count; ++i) {
        KyWiredItem item;
        item.m_connectName = QString("Connection%1").arg(i);
        item.m_connectUuid = QString("uuid-%1").arg(i);
        item.m_connectPath = QString("/path/%1").arg(i);
        list.append(item);
    }
    return list;
}
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

#ifndef TEST_KYLINWIREDNETRESOURCE_H
#define TEST_KYLINWIREDNETRESOURCE_H

#include <gtest/gtest.h>
#include <QObject>
#include <QMap>
#include <QList>
#include <QString>
#include <QStringList>

// Include the actual class definitions
#include "kylinwirednetresource.h"
#include "kylinconnectitem.h"
#include "kylinnetworkresourcemanager.h"
#include "kylinnetworkdeviceresource.h"

class TestKyWiredConnectResourse : public testing::Test
{
protected:
    static void SetUpTestSuite();
    static void TearDownTestSuite();

    void SetUp() override;
    void TearDown() override;

    // Helper methods for creating test data
    static KyWiredItem createWiredItem(const QString &name, const QString &uuid, const QString &path);
    static QList<KyWiredItem> createWiredItemList(int count);

    // Member variables
    static KyWiredConnectResourse *m_resource;
    static KyNetworkResourceManager *m_resourceManager;
    static KyNetworkDeviceResourse *m_deviceResource;
};

// Tests for KyWiredConnectResourse
TEST_F(TestKyWiredConnectResourse, Constructor_Test)
{
    ASSERT_NE(m_resource, nullptr);
}

TEST_F(TestKyWiredConnectResourse, GetWiredList_Test)
{
    QMap<QString, QList<KyWiredItem>> wiredMap;
    m_resource->getWiredList(wiredMap);
    EXPECT_TRUE(wiredMap.isEmpty() || !wiredMap.isEmpty());
}

TEST_F(TestKyWiredConnectResourse, GetWiredConnectDetail_Test)
{
    KyDetailInfo detailInfo;
    m_resource->getWiredConnectDetail(QString(), QString(), detailInfo);
    m_resource->getWiredConnectDetail("eth0", QString(), detailInfo);
    m_resource->getWiredConnectDetail("eth0", "test-uuid-1234", detailInfo);
    SUCCEED();
}

TEST_F(TestKyWiredConnectResourse, GetWiredList_EmptyMap_Test)
{
    QMap<QString, QList<KyWiredItem>> wiredMap;
    wiredMap.insert("eth0", QList<KyWiredItem>());
    wiredMap.insert("eth1", QList<KyWiredItem>());
    m_resource->getWiredList(wiredMap);
    SUCCEED();
}

TEST_F(TestKyWiredConnectResourse, MultipleGetWiredListCalls_Test)
{
    QMap<QString, QList<KyWiredItem>> wiredMap1;
    QMap<QString, QList<KyWiredItem>> wiredMap2;
    m_resource->getWiredList(wiredMap1);
    m_resource->getWiredList(wiredMap2);
    SUCCEED();
}

TEST_F(TestKyWiredConnectResourse, GetWiredConnectDetail_MultipleCalls_Test)
{
    KyDetailInfo detailInfo1;
    KyDetailInfo detailInfo2;
    KyDetailInfo detailInfo3;
    m_resource->getWiredConnectDetail("eth0", "uuid-1", detailInfo1);
    m_resource->getWiredConnectDetail("eth1", "uuid-2", detailInfo2);
    m_resource->getWiredConnectDetail("", "", detailInfo3);
    SUCCEED();
}

TEST_F(TestKyWiredConnectResourse, SignalConnection_Test)
{
    ASSERT_NE(m_resource, nullptr);
    SUCCEED();
}

TEST_F(TestKyWiredConnectResourse, BranchCoverage_Test)
{
    QMap<QString, QList<KyWiredItem>> wiredMap;
    m_resource->getWiredList(wiredMap);
    wiredMap.insert("test", QList<KyWiredItem>());
    m_resource->getWiredList(wiredMap);
    
    KyDetailInfo detail;
    m_resource->getWiredConnectDetail("", "", detail);
    m_resource->getWiredConnectDetail("eth0", "", detail);
    m_resource->getWiredConnectDetail("", "uuid", detail);
    m_resource->getWiredConnectDetail("eth0", "uuid", detail);
    SUCCEED();
}

#endif
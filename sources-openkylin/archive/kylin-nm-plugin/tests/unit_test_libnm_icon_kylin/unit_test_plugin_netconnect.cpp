/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
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
#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>

#include "../../plugins/netconnect/netconnect.h"

#include "stubext.h"
using namespace stub_ext;

class NetConnectTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        PluginType type = PluginType::SIMPLE;
        m_pNetConnectTest = new NetConnect();
    }

    static void TearDownTestSuite()
    {
        if (m_pNetConnectTest) {
            delete m_pNetConnectTest;
            m_pNetConnectTest = nullptr;
        }
    }

    static NetConnect *m_pNetConnectTest;
};

NetConnect *NetConnectTest::m_pNetConnectTest = nullptr;

TEST_F(NetConnectTest, testAllFunc)
{

    //    QWidget *m_widget = m_pNetConnectTest->pluginUi();
    PluginType mtype = PluginType::SIMPLE;
    m_pNetConnectTest->setPluginType(mtype, true);

    //    //    m_pNetConnectTest->setParentWidget(nullptr);
    //    m_pNetConnectTest->setWidgetVisable(true);

    //    m_pNetConnectTest->initUi();
    //    m_pNetConnectTest->initComponent();
    //    m_pNetConnectTest->initConnect();
    //    m_pNetConnectTest->runExternalApp();

    //    // 开关相关
    //    m_pNetConnectTest->setSwitchStatus();
    //    QVBoxLayout *m_vboxLayoutTest = new QVBoxLayout();
    //    m_pNetConnectTest->hideLayout(m_vboxLayoutTest);
    //    m_pNetConnectTest->showLayout(m_vboxLayoutTest);

    //    QString m_connName = "test";
    //    m_pNetConnectTest->getInsertPos(m_connName, m_vboxLayoutTest);
    //    QString m_uuid = "9999-0000-0000-1111-2222";
    //    QString m_deviceName = "Lan0";
    //    m_pNetConnectTest->activeConnect(m_uuid, m_deviceName);
    //    //    m_pNetConnectTest->deActiveConnect(m_uuid);

    //    // 获取设备列表
    //    QMap<QString, bool> m_map;
    //    m_pNetConnectTest->getDeviceStatusMap(m_map);
    //    m_pNetConnectTest->initNet();
    //    m_pNetConnectTest->initNetListFromDevice(m_deviceName);
    //    // 处理列表增加
    //    ItemFrame *m_itemFrame = new ItemFrame(m_deviceName);
    //    //    KyWiredItem m_kyWiredItem;
    //    //    m_pNetConnectTest->addLanItem(m_itemFrame, m_deviceName, m_kyWiredItem);
    //    // 增加设备
    //    m_pNetConnectTest->addDeviceFrame(m_deviceName, true);
    //    // 减少设备
    //    m_pNetConnectTest->removeDeviceFrame(m_deviceName);
    //    // 增加一项
    //    QStringList m_infoList;
    //    m_pNetConnectTest->addOneLanFrame(m_itemFrame, m_deviceName, m_infoList);
    //    // 减少一项
    //    m_pNetConnectTest->removeOneLanFrame(m_itemFrame, m_deviceName, m_uuid);

    //    // 单个lan连接状态变化
    //    LanItem *m_lanItem = new LanItem(true);
    //    KyConnectState m_kyConnectState = KyConnectState::CONNECT_STATE_ACTIVATING;
    //    m_pNetConnectTest->itemActiveConnectionStatusChanged(m_lanItem, m_kyConnectState);

    //    //    QObject *m_wObject = new QObject();
    //    //    QEvent *m_event;
    //    //    bool status = m_pNetConnectTest->eventFilter(m_wObject, m_event);
    //    //    qDebug() << Q_FUNC_INFO << __LINE__ << status;

    //    QString m_connectUuid;
    //    QString m_connectName;
    //    QString m_connectPath;
    //    m_pNetConnectTest->updateLanInfo(m_deviceName, m_connectUuid, m_connectName, m_connectPath);

    //    m_pNetConnectTest->onLanAdd(m_deviceName, m_connectUuid, m_connectName, m_connectPath);
    //    QString m_dbusPath;
    //    m_pNetConnectTest->onLanRemove(m_dbusPath);

    //    m_pNetConnectTest->onActiveConnectionChanged(m_deviceName, m_uuid, m_kyConnectState);

    //    //    void onDeviceStatusChanged();
    //    m_pNetConnectTest->onDeviceNameChanged(m_deviceName, m_deviceName);
    //    m_pNetConnectTest->onDeviceCarrierChange(m_deviceName, true);

    //    m_pNetConnectTest->onDeviceManagedChange(m_deviceName, true);
    //    m_pNetConnectTest->onWiredEnabledChanged(true);

    //    m_pNetConnectTest->onDeviceAdd(m_deviceName);
    //    m_pNetConnectTest->onDeviceRemove(m_deviceName);
}

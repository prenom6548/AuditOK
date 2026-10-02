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

#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>
#include <QDBusReply>
#include <QThread>
#include <QObject>
#include "../../libnm-kylin/libnm-base-kylin/kylinnetworkmanager.h"
#include "stubext.h"

using namespace stub_ext;

class NetWiredTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        thread = new QThread;
		manager = new KyNetworkManager();
		manager->moveToThread(thread);
        QObject::connect(thread, &QThread::started, manager, &KyNetworkManager::kylinNetworkManagerInit);
        QObject::connect(thread, &QThread::finished, manager, &KyNetworkManager::deleteLater);
        QObject::connect(thread, &QThread::finished, [=](){
			qDebug() << "NetConnect thread quit";
		});
		thread->start();
        sleep(2);
    }

    static void TearDownTestSuite()
    {
	if (thread) {
			thread->quit();
			thread->wait();
			delete thread;
	}

    }

    static KyNetworkManager *manager;
    static QThread *thread;
   static KyWiredConnectOperation* moperate;

};

KyNetworkManager *NetWiredTest::manager = nullptr;
QThread *NetWiredTest::thread = nullptr;
KyWiredConnectOperation *NetWiredTest::moperate=nullptr;
TEST_F(NetWiredTest, getWireDevice)
{
    KyConnectStatus connectType;
    QMap<QString,QList<KyWiredItem>> map;
    if (!manager) {
       return;
    }
   bool bEnable=manager->getNetworkingEnabled();
    EXPECT_TRUE(bEnable);
    bEnable=manager->getWiredEnabled();
    manager->getConnectStatus(connectType);
    //ASSERT_EQ(newValue, false);
    QStringList wiredDevList;
    wiredDevList.clear();
    manager->getNetworkDeviceList(DEVICE_TYPE_ETHERNET, wiredDevList);
    EXPECT_FALSE(wiredDevList.isEmpty());

	if (!wiredDevList.isEmpty())
	{
        for (int i = 0; i < wiredDevList.size(); ++i)
        {
            KyConnectivity connectivity;
            QString devName = wiredDevList.at(i);

            KyWirelessConnectSetting connetSetting;
            connetSetting.setIfaceName(devName);
            connetSetting.setIpConfigType(IPADDRESS_V4, CONFIG_IP_DHCP);
            connetSetting.m_connectName=QString("mqtest");

            moperate->createWiredConnect(connetSetting);

            connetSetting.m_connectName=QString("mqtest1");
            manager->onCreateWiredConnect(connetSetting);

            connetSetting.m_connectName=QString("mqtest2");
            manager->onCreateWiredConnect(connetSetting);


            qDebug() << devName;
            if( manager->getDeviceManaged(devName))
            {
               manager->getDeviceState(devName);
               manager->getDeviceConnectivity(devName,connectivity);
            }
            manager->onSetWiredDeviceEnable(devName,true);
            manager->onSetDeviceManaged(devName,true);

            manager->getWiredDeviceIsCarriered(devName);

		}
	}

    manager->getWiredDeviceIsCarriered("TESTNAME");
    manager->wiredConnectIsActived();
}

TEST_F(NetWiredTest, WireConnect)
{
    QMap<QString,QList<KyWiredItem>> map;
    KyConnectOperation operate;
    KyConnectSetting connectSetting;
    KyDetailInfo detailConnectSetting;
    QList<KyActivateItem> connectItemList;
    if (!manager) {
       return;
    }
    manager->getWiredList(map);

    EXPECT_FALSE(map.isEmpty());
    QMap<QString, QList<KyWiredItem> >::iterator iter = map.begin();
    while (iter != map.end()) {

       QList<KyWiredItem>listCon =iter.value();
       manager->getActiveConnectionList(iter.key(),CONNECT_TYPE_WIRED, connectItemList);
        qDebug() <<"device name "<<iter.key();
       for (int i = 0; i < listCon.length(); i++) {
           qDebug() << "[NetConnect]m_connectName " << listCon.at(i).m_connectName;
           manager->getWiredConnectDetail(iter.key(),listCon.at(i).m_connectUuid,detailConnectSetting);
           manager->getConnectIpInfo(listCon.at(i).m_connectUuid,connectSetting);
           if(listCon.at(i).m_connectName==QString("mqtest"))
           {
               manager->onDeactivateConnection(listCon.at(i).m_connectUuid);
               manager->onDeleteConnect(listCon.at(i).m_connectUuid);
               qDebug() << "unit test delete " << listCon.at(i).m_connectName;
               break;
           }
       }
        iter++;
    }
}

TEST_F(NetWiredTest, getWirelessList)
{
    KyConnectStatus connectType;
    KyConnectSetting connectSetting;
     KyDetailInfo detailConnectSetting;
     KyWpaPasswordInfo wpainfo;
     KyEapMethodLeapInfo leapinfo;
     KyEapMethodPeapInfo peapinfo;
     KyEapMethodTtlsInfo ttlsinfo;
    if (!manager) {
       return;
    }
    bool bEnable=manager->getNetworkingEnabled();
     EXPECT_TRUE(bEnable);
     bEnable=manager->getWirelessEnabled();
     manager->getConnectStatus(connectType);


    manager->onRequestWirelessScan();
    //ASSERT_EQ(newValue, false);
    QStringList wirelessDevList;
    wirelessDevList.clear();
    manager->getNetworkDeviceList(DEVICE_TYPE_WIFI, wirelessDevList);

    QList<KyWirelessNetItem> list;
    manager->getWifiNetworkList("UNKOWN_TEST", list);

    manager->getAcivateWifiSignal();

    qDebug() << wirelessDevList;
    if (!wirelessDevList.isEmpty())
    {
        
        for (int i = 0; i < wirelessDevList.size(); ++i)
        {
            QString devName = wirelessDevList.at(i);
            qDebug() << devName;
            if( manager->getDeviceManaged(devName))
            {
               manager->getWifiNetworkList(devName, list);
               if(!list.isEmpty())
               {
                 manager->getWirelessConnectDetail(devName, list.at(0).m_NetSsid,list.at(0).m_connectUuid,detailConnectSetting);
                 QString uuid=list.at(0).m_connectUuid;

                 manager->getNormalWifiConnectSecuInfo(uuid, wpainfo);

                 manager->getEnterPriseInfoPeap(uuid, peapinfo);

                 manager->getEnterPriseInfoTtls(uuid, ttlsinfo);

                 manager->getEnterPriseInfoLeap(uuid, leapinfo);



                 qDebug() << list.at(0).m_NetSsid<<list.at(0).m_dbusPath;
                 manager->getConnectIpInfo(list.at(0).m_connectUuid,connectSetting);
               }
            }
        }
    }
}

TEST_F(NetWiredTest, getWirelessConInfo)
{
    KyConnectStatus connectType;
    KyConnectSetting connectSetting;
     KyDetailInfo detailConnectSetting;
     KyEapMethodTlsInfo tlsInfo;
     KyWpaPasswordInfo wpainfo;
     KyEapMethodLeapInfo leapinfo;
     KyEapMethodPeapInfo peapinfo;
     KyEapMethodTtlsInfo ttlsinfo;
    KyEapMethodPwdInfo pwdInfo;
    KyEapMethodFastInfo fastInfo;

    if (!manager) {
       return;
    }

    QString uuid = "00000000-0000-0000-0000-000000000000";
    QString ssid = "TestSsid";
    QList<KyWirelessNetItem> list;
    QString devName="wlp2s0";

    manager->getWirelessConnectDetail(devName, ssid,uuid,detailConnectSetting);
    
    manager->getNormalWifiConnectSecuInfo(uuid, wpainfo);

    manager->getEnterPriseInfoTls(uuid, tlsInfo);

    manager->getEnterPriseInfoPeap(uuid, peapinfo);

    manager->getEnterPriseInfoTtls(uuid, ttlsinfo);

    manager->getEnterPriseInfoLeap(uuid, leapinfo);

    manager->getConnectIpInfo(uuid,connectSetting);

    manager->getEnterPriseInfoPwd(uuid, pwdInfo);

    manager->getEnterPriseInfoFast(uuid,fastInfo);

    QList<KyApConnectItem> apConnectItemList;
    // Test with empty connection list
    manager->getApConnections(apConnectItemList);

        //获取企业网类型
    KyEapMethodType type;
    manager->getEnterpiseEapMethod(uuid, type);

    manager->wirelessConnectIsActived();
}

TEST_F(NetWiredTest, setNetwork)
{
    QString uuid = "00000000-0000-0000-0000-000000000000";
    QString deviceName="TEST_DEVICE";
    if (!manager) {
       return;
    }

    manager->onSetWirelessNetworkEnabled(true);

    manager->onSetNetworkingEnabled(true);
    manager->onSetWiredEnabled(true);

    KyWirelessConnectSetting connetSetting;
    connetSetting.setIfaceName(deviceName);
    connetSetting.setIpConfigType(IPADDRESS_V4, CONFIG_IP_DHCP);
    connetSetting.m_connectName=QString("mqUnitTest");
    manager->onCreateWiredConnect(connetSetting);

    //有线无线公用
    manager->onDeleteConnect(uuid);

    manager->onActivateConnection(uuid, deviceName);

    manager->onDeactivateConnection(uuid);
    //wireless
    //申请扫描

    connetSetting.m_ssid=QString("mqUnitTest");
    manager->onAddAndActivateNormalWifi(connetSetting, KySecuType::WPA_AND_WPA2_PERSONAL);
    //连接无本地配置的企业网热点
    KyEapMethodTlsInfo info;
    info.identity = "test_identity";
    info.domain = "test.domain.com";
    info.devIfaceName = "wlan0";
    info.caCertPath = "/path/to/ca.crt";
    info.bNeedCa = false;
    info.clientCertPath = "/path/to/client.crt";
    info.clientPrivateKey = "/path/to/client.key";
    info.clientPrivateKeyPWD = "password";
    info.m_privateKeyPWDFlag = NetworkManager::Setting::None;
    info.bChanged = false;
    manager->onAddAndActiveWirelessEnterPriseTlsConnect(info,connetSetting);

    KyEapMethodPeapInfo peapInfo;
    peapInfo.phase2AuthMethod = KyAuthMethodMschapv2;
    peapInfo.userName = "test_user";
    peapInfo.userPWD = "test_password";
    peapInfo.m_passwdFlag = NetworkManager::Setting::None;
    manager->onAddAndActiveWirelessEnterPrisePeapConnect(peapInfo,connetSetting);


    KyEapMethodTtlsInfo ttlsInfo;
    ttlsInfo.authType = AUTH_EAP;
    ttlsInfo.authEapMethod = KyAuthEapMethodMschapv2;
    ttlsInfo.userName = "test_user";
    ttlsInfo.userPWD = "test_password";
    ttlsInfo.m_passwdFlag = NetworkManager::Setting::None;
    ttlsInfo.bChanged = false;
    manager->onAddAndActiveWirelessEnterPriseTtlsConnect(ttlsInfo,connetSetting);

    KyEapMethodLeapInfo leapInfo;
    leapInfo.m_userName = "test_user";
    leapInfo.m_userPwd = "test_password";
    leapInfo.m_passwdFlag = NetworkManager::Setting::None;
    leapInfo.bChanged = false;
    manager->onAddAndActiveWirelessEnterPriseLeapConnect(leapInfo,connetSetting);

    KyEapMethodPwdInfo pwdInfo;
    pwdInfo.m_userName = "test_user";
    pwdInfo.m_userPwd = "test_password";
    pwdInfo.m_passwdFlag = NetworkManager::Setting::None;
    pwdInfo.bChanged = false;
    manager->onAddAndActiveWirelessEnterPrisePwdConnect(pwdInfo,connetSetting);

    KyEapMethodFastInfo fastInfo;
    fastInfo.m_anonIdentity = "anonymous";
    fastInfo.m_pacProvisioning = KyFastProvisioningAllowAuthenticated;
    fastInfo.m_allowAutoPacFlag = true;
    fastInfo.m_pacFilePath = "/path/to/pac file";
    fastInfo.m_authMethod = KyAuthMethodMschapv2;
    fastInfo.m_userName = "test_user";
    fastInfo.m_userPwd = "test_password";
    fastInfo.m_passwdFlag = NetworkManager::Setting::None;
    fastInfo.bChanged = false;
    manager->onAddAndActiveWirelessEnterPriseFastConnect(fastInfo,connetSetting);

    //=====================新增配置操作====================
    //新增普通连接
    KySecuType type=WPA_AND_WPA2_PERSONAL;
    manager->onAddNormalConnect(connetSetting, type);
    //新增TLS连接
     manager->onAddTlsConnect(connetSetting,info);
    //新增PEAP连接
    manager->onAddPeapConnect(connetSetting, peapInfo);
    //新增TTLS连接
    manager->onAddTtlsConnect(connetSetting,ttlsInfo);

    //=====================更新配置操作====================
    manager->onUpdateIpv4AndIpv6SettingInfo(uuid,connetSetting);
    //连接修改(安全改为个人/None)
    manager->onUpdateWirelessPersonalConnect(uuid, connetSetting, true);
    //连接修改(安全改为改为企业)
    manager->onUpdateWirelessEnterPriseTlsConnect(uuid, info);
    manager->onUpdateWirelessEnterPrisePeapConnect(uuid,peapInfo);
    manager->onUpdateWirelessEnterPriseTtlsConnect(uuid, ttlsInfo);
    manager->onUpdateWirelessEnterPriseLeapConnect(uuid, leapInfo);
    manager->onUpdateWirelessEnterPrisePwdConnect(uuid, pwdInfo);

    manager->onUpdateWirelessEnterPriseFastConnect(uuid, fastInfo);

    manager->onUpdateWirelessAutoConnectState(uuid,true);

}

TEST_F(NetWiredTest,test_KyApNetResource)
{

    KyApConnectItem *item = new KyApConnectItem();
    if(item) delete item;

    KyWirelessNetItem *wirelessItem=new KyWirelessNetItem();
    if(wirelessItem) delete wirelessItem;

    KyApNetResource *apResource = new KyApNetResource();
    if(!apResource){
        return;
    }
    QList<KyApConnectItem> apConnectItemList;

    // Test with empty connection list
    apResource->getApConnections(apConnectItemList);
    delete apResource;

}

TEST_F(NetWiredTest, test_KyNetworkDeviceResourse)
{
    KyNetworkDeviceResourse *pDeviceResourse=new KyNetworkDeviceResourse();
    if(!pDeviceResourse || !manager){
        return;
    }
    QString hardAddress;
    int bandWith;
    QStringList wiredDevList;
    wiredDevList.clear();
    manager->getNetworkDeviceList(DEVICE_TYPE_ETHERNET, wiredDevList);
    EXPECT_FALSE(wiredDevList.isEmpty());
    QString devName = wiredDevList.at(0);
     pDeviceResourse->getHardwareInfo(devName, hardAddress,bandWith);

    //移动热点有关
    QString strMac;
    uint iHz;
    uint iChan;
    QString secuType;
    pDeviceResourse->getDeviceActiveAPInfo(devName, strMac,iHz, iChan, secuType);
     pDeviceResourse->getWirelessDeviceCapability(devName);

     pDeviceResourse->wiredDeviceIsCarriered(devName);
     pDeviceResourse->wirelessDeviceIsExist(devName);
     pDeviceResourse->setDeviceManaged(devName, true);
     pDeviceResourse->getDeviceManaged(devName);


}

TEST_F(NetWiredTest,test_updateConnect)
{
    QMap<QString,QList<KyWiredItem>> map;
    KyConnectOperation *m_connectOperation = new KyConnectOperation(manager);
    QList<KyActivateItem> connectItemList;
    if (!manager || !m_connectOperation) {
       printf("init failed\n");
       return;
    }
    
    KyConnectSetting connectSettings;
    QString invalidUuid = "00000000-0000-0000-0000-000000000000";
    m_connectOperation->updateConnect(invalidUuid, connectSettings);
    m_connectOperation->activateConnection(invalidUuid, "eth0");
    m_connectOperation->deactivateConnection(invalidUuid);
    m_connectOperation->deleteConnect(invalidUuid);
    
    manager->getWiredList(map);
    EXPECT_FALSE(map.isEmpty());
    QMap<QString, QList<KyWiredItem> >::iterator iter = map.begin();
    while (iter != map.end()) {

        printf("in map\n");
        manager->getActiveConnectionList(iter.key(),CONNECT_TYPE_WIRED, connectItemList);
       QList<KyWiredItem>listCon =iter.value();
       for (int i = 0; i < listCon.length(); i++) {

           printf("in m_connectName %s  size %d\n",listCon.at(i).m_connectName.toStdString().c_str(),listCon.length());
           if(listCon.at(i).m_connectName.contains("mqtest"))
           {
                manager->getConnectIpInfo(listCon.at(i).m_connectUuid,connectSettings);
                m_connectOperation->updateConnect(listCon.at(i).m_connectUuid, connectSettings);
                sleep(2);
                m_connectOperation->deactivateConnection(connectItemList.at(0).m_uuid);
                sleep(2);
                m_connectOperation->activateConnection(listCon.at(i).m_connectUuid, iter.key() );
                sleep(2);
                 m_connectOperation->activateConnection(connectItemList.at(0).m_uuid, iter.key() );
                sleep(2);
                manager->onUpdateWirelessAutoConnectState(connectItemList.at(0).m_uuid,true);
               qDebug() << "test_updateConnect " << listCon.at(i).m_connectName<<connectItemList.at(0).m_connName;
               break;
           }
       }
        iter++;
    }
    if (m_connectOperation) {
        delete m_connectOperation;
        m_connectOperation = nullptr;
        printf("delete \n");
    }
}

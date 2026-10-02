#ifndef TEST_KYWIRELESSNETRESOURCE_H
#define TEST_KYWIRELESSNETRESOURCE_H

#include <QObject>
#include <QString>
#include <QSignalSpy>
#include <QTest>
#include "kywirelessnetresource.h"

class TestKyWirelessNetResource : public QObject
{
    Q_OBJECT

public:
    explicit TestKyWirelessNetResource(QObject *parent = nullptr);
    ~TestKyWirelessNetResource();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void test_constructor();
    void test_getWifiNetworkList_emptyList();
    void test_onWifiNetworkAdded();
    void test_onWifiNetworkRemoved();

    void test_getWifiNetworkList_multipleDevices();
    void test_getNormalWifiConnectSecuInfo();
    void test_getNormalWifiConnectSecuInfo_invalidUuid();
    void test_getNormalWifiConnectSecuInfo_withValidData();
    void test_getEnterpiseEapMethod_nullConnection();
    void test_getEnterpiseEapMethod_notWpaEap();
    void test_getEnterpiseEapMethod_emptyUuid();
    void test_getEnterPriseInfoTls_nullConnection();
    void test_getEnterPriseInfoTls_nullSecuritySetting();
    void test_getEnterPriseInfoTls_invalidKeyMgmt();
    void test_getEnterPriseInfoTls_emptyUuid();
    void test_getEnterPriseInfoPeap_nullConnection();
    void test_getEnterPriseInfoPeap_nullSecuritySetting();
    void test_getEnterPriseInfoPeap_invalidKeyMgmt();
    void test_getEnterPriseInfoPeap_emptyUuid();
    void test_getEnterPriseInfoTtls_nullConnection();
    void test_getEnterPriseInfoTtls_nullSecuritySetting();
    void test_getEnterPriseInfoTtls_invalidKeyMgmt();
    void test_getEnterPriseInfoTtls_emptyUuid();
    void test_getEnterPriseInfoLeap_nullConnection();
    void test_getEnterPriseInfoLeap_nullSecuritySetting();
    void test_getEnterPriseInfoLeap_invalidKeyMgmt();
    void test_getEnterPriseInfoLeap_emptyUuid();
    void test_getEnterPriseInfoPwd_nullConnection();
    void test_getEnterPriseInfoPwd_nullSecuritySetting();
    void test_getEnterPriseInfoPwd_invalidKeyMgmt();
    void test_getEnterPriseInfoPwd_emptyUuid();
    void test_getEnterPriseInfoFast_nullConnection();
    void test_getEnterPriseInfoFast_nullSecuritySetting();
    void test_getEnterPriseInfoFast_invalidKeyMgmt();
    void test_getEnterPriseInfoFast_emptyUuid();
    void test_getWirelessConnectDetail();
    void test_getWirelessConnectDetail_emptyInterface();
    void test_getWirelessConnectDetail_emptySsid();
    void test_getWirelessConnectDetail_emptyUuid();
    void test_onWifiNetworkPropertyChange();
    void test_onWifiNetworkDeviceDisappear();
    void test_onConnectionAdd();
    void test_onConnectionRemove();
    void test_onDeviceAdd();
    void test_onDeviceNameUpdate();
    void test_onDeviceRemove();
    void test_onDeviceNameUpdate_invalidDevice();
    void test_kyWirelessNetItemListInit();
    void test_wirelessNetItemInit();
    void test_getUuidBySsid();
    void test_updateList();
    void test_signalWifiNetworkUpdate();
    void test_signalWirelessConnectionUpdate();
    void test_KyWirelessNetItem_struct();
    void test_KyWpaPasswordInfo_struct();
    void test_KyDetailInfo_struct();
    void test_getDeviceIFace();

private:
    KyWirelessNetResource *m_wirelessResource = nullptr;
    int m_TestMode=0;
};

#endif // TEST_KYWIRELESSNETRESOURCE_H

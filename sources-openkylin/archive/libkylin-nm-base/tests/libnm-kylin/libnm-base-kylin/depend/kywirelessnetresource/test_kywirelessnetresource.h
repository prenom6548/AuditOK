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

private slots:
    void initTestCase();
    void cleanupTestCase();

    void test_constructor();
    void test_getWifiNetworkList_emptyList();
    void test_getWifiNetworkList_withValidDevice();
    void test_getNormalWifiConnectSecuInfo();
    void test_getEnterpiseEapMethod_nullConnection();
    void test_getEnterpiseEapMethod_notWpaEap();
    void test_getEnterPriseInfoTls_nullConnection();
    void test_getEnterPriseInfoTls_nullSecuritySetting();
    void test_getEnterPriseInfoTls_invalidKeyMgmt();
    void test_getEnterPriseInfoPeap_nullConnection();
    void test_getEnterPriseInfoPeap_nullSecuritySetting();
    void test_getEnterPriseInfoPeap_invalidKeyMgmt();
    void test_getEnterPriseInfoTtls_nullConnection();
    void test_getEnterPriseInfoTtls_nullSecuritySetting();
    void test_getEnterPriseInfoTtls_invalidKeyMgmt();
    void test_getEnterPriseInfoLeap_nullConnection();
    void test_getEnterPriseInfoLeap_nullSecuritySetting();
    void test_getEnterPriseInfoLeap_invalidKeyMgmt();
    void test_getEnterPriseInfoPwd_nullConnection();
    void test_getEnterPriseInfoPwd_nullSecuritySetting();
    void test_getEnterPriseInfoPwd_invalidKeyMgmt();
    void test_getEnterPriseInfoFast_nullConnection();
    void test_getEnterPriseInfoFast_nullSecuritySetting();
    void test_getEnterPriseInfoFast_invalidKeyMgmt();
    void test_getWirelessConnectDetail();

private:
    KyWirelessNetResource *m_wirelessResource = nullptr;
};

#endif // TEST_KYWIRELESSNETRESOURCE_H

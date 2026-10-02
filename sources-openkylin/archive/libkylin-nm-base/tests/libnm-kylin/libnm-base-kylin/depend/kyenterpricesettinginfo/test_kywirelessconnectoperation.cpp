#include "test_kywirelessconnectoperation.h"

#include <NetworkManagerQt/Ipv4Setting>
#include <NetworkManagerQt/Ipv6Setting>
#include <NetworkManagerQt/ConnectionSettings>
#include "kylinconnectsetting.h"

TestKyWirelessConnectOperation::TestKyWirelessConnectOperation(QObject *parent)
    : QObject(parent)
    , m_wirelessConnectOperation(nullptr)
{
}

TestKyWirelessConnectOperation::~TestKyWirelessConnectOperation()
{
}

void TestKyWirelessConnectOperation::initTestCase()
{
}

void TestKyWirelessConnectOperation::cleanupTestCase()
{
}

void TestKyWirelessConnectOperation::init()
{
    m_wirelessConnectOperation = new KyWirelessConnectOperation();
}

void TestKyWirelessConnectOperation::cleanup()
{
    if (m_wirelessConnectOperation) {
        delete m_wirelessConnectOperation;
        m_wirelessConnectOperation = nullptr;
    }
}

void TestKyWirelessConnectOperation::test_constructor()
{
    KyWirelessConnectOperation *operation = new KyWirelessConnectOperation();
    QVERIFY(operation != nullptr);
    delete operation;
}

void TestKyWirelessConnectOperation::test_destructor()
{
    KyWirelessConnectOperation *operation = new KyWirelessConnectOperation();
    delete operation;
    QVERIFY(true);
}

void TestKyWirelessConnectOperation::test_requestWirelessScan()
{
    m_wirelessConnectOperation->requestWirelessScan();
    QVERIFY(true);
}

void TestKyWirelessConnectOperation::test_getConnSecretFlags_notFound()
{
    QString invalidUuid = "00000000-0000-0000-0000-000000000000";
    NetworkManager::Setting::SecretFlags flag;
    bool result = m_wirelessConnectOperation->getConnSecretFlags(invalidUuid, flag);
    QVERIFY(result == false);
}

void TestKyWirelessConnectOperation::test_getPsk_notFound()
{
    QString invalidUuid = "00000000-0000-0000-0000-000000000000";
    QString psk = m_wirelessConnectOperation->getPsk(invalidUuid);
    QVERIFY(psk == QString(""));
}

void TestKyWirelessConnectOperation::test_getPrivateKeyPassword_notFound()
{
    QString invalidUuid = "00000000-0000-0000-0000-000000000000";
    QString password = m_wirelessConnectOperation->getPrivateKeyPassword(invalidUuid);
    QVERIFY(password == QString(""));
}

void TestKyWirelessConnectOperation::test_get8021xPassword_notFound()
{
    QString invalidUuid = "00000000-0000-0000-0000-000000000000";
    QString password = m_wirelessConnectOperation->get8021xPassword(invalidUuid);
    QVERIFY(password == QString(""));
}

void TestKyWirelessConnectOperation::test_setWirelessAutoConnect_notFound()
{
    QSignalSpy spy(m_wirelessConnectOperation, &KyWirelessConnectOperation::updateConnectionError);
    
    QString invalidUuid = "00000000-0000-0000-0000-000000000000";
    m_wirelessConnectOperation->setWirelessAutoConnect(invalidUuid, true);
    
    QVERIFY(spy.count() == 1);
}

void TestKyWirelessConnectOperation::test_updateIpv4AndIpv6SettingInfo_notFound()
{
    QSignalSpy spy(m_wirelessConnectOperation, &KyWirelessConnectOperation::updateConnectionError);
    
    QString invalidUuid = "00000000-0000-0000-0000-000000000000";
    KyConnectSetting connectSettings;
    m_wirelessConnectOperation->updateIpv4AndIpv6SettingInfo(invalidUuid, connectSettings);
    
    QVERIFY(spy.count() == 1);
}

void TestKyWirelessConnectOperation::test_updateWirelessPersonalConnect_notFound()
{
    QSignalSpy spy(m_wirelessConnectOperation, &KyWirelessConnectOperation::updateConnectionError);
    
    QString invalidUuid = "00000000-0000-0000-0000-000000000000";
    KyWirelessConnectSetting connSettingInfo;
    m_wirelessConnectOperation->updateWirelessPersonalConnect(invalidUuid, connSettingInfo, false);
    
    QVERIFY(spy.count() == 1);
}

void TestKyWirelessConnectOperation::test_updateWirelessEnterPriseTlsConnect_notFound()
{
    QSignalSpy spy(m_wirelessConnectOperation, &KyWirelessConnectOperation::updateConnectionError);
    
    QString invalidUuid = "00000000-0000-0000-0000-000000000000";
    KyEapMethodTlsInfo tlsInfo;
    m_wirelessConnectOperation->updateWirelessEnterPriseTlsConnect(invalidUuid, tlsInfo);
    
    QVERIFY(spy.count() == 1);
}

void TestKyWirelessConnectOperation::test_updateWirelessEnterPrisePeapConnect_notFound()
{
    QSignalSpy spy(m_wirelessConnectOperation, &KyWirelessConnectOperation::updateConnectionError);
    
    QString invalidUuid = "00000000-0000-0000-0000-000000000000";
    KyEapMethodPeapInfo peapInfo;
    m_wirelessConnectOperation->updateWirelessEnterPrisePeapConnect(invalidUuid, peapInfo);
    
    QVERIFY(spy.count() == 1);
}

void TestKyWirelessConnectOperation::test_updateWirelessEnterPriseTtlsConnect_notFound()
{
    QSignalSpy spy(m_wirelessConnectOperation, &KyWirelessConnectOperation::updateConnectionError);
    
    QString invalidUuid = "00000000-0000-0000-0000-000000000000";
    KyEapMethodTtlsInfo ttlsInfo;
    m_wirelessConnectOperation->updateWirelessEnterPriseTtlsConnect(invalidUuid, ttlsInfo);
    
    QVERIFY(spy.count() == 1);
}

void TestKyWirelessConnectOperation::test_updateWirelessEnterPriseLeapConnect_notFound()
{
    QSignalSpy spy(m_wirelessConnectOperation, &KyWirelessConnectOperation::updateConnectionError);
    
    QString invalidUuid = "00000000-0000-0000-0000-000000000000";
    KyEapMethodLeapInfo leapInfo;
    m_wirelessConnectOperation->updateWirelessEnterPriseLeapConnect(invalidUuid, leapInfo);
    
    QVERIFY(spy.count() == 1);
}

void TestKyWirelessConnectOperation::test_updateWirelessEnterPrisePwdConnect_notFound()
{
    QSignalSpy spy(m_wirelessConnectOperation, &KyWirelessConnectOperation::updateConnectionError);
    
    QString invalidUuid = "00000000-0000-0000-0000-000000000000";
    KyEapMethodPwdInfo pwdInfo;
    m_wirelessConnectOperation->updateWirelessEnterPrisePwdConnect(invalidUuid, pwdInfo);
    
    QVERIFY(spy.count() == 1);
}

void TestKyWirelessConnectOperation::test_updateWirelessEnterPriseFastConnect_notFound()
{
    QSignalSpy spy(m_wirelessConnectOperation, &KyWirelessConnectOperation::updateConnectionError);
    
    QString invalidUuid = "00000000-0000-0000-0000-000000000000";
    KyEapMethodFastInfo fastInfo;
    m_wirelessConnectOperation->updateWirelessEnterPriseFastConnect(invalidUuid, fastInfo);
    
    QVERIFY(spy.count() == 1);
}

void TestKyWirelessConnectOperation::test_addConnect_notFound()
{
    QSignalSpy spy(m_wirelessConnectOperation, &KyWirelessConnectOperation::createConnectionError);
    
    KyWirelessConnectSetting connSettingInfo;
    connSettingInfo.m_ssid = "TestSSID";
    connSettingInfo.m_ifaceName = "wlan0";
    KySecuType type = NONE;
    
    m_wirelessConnectOperation->addConnect(connSettingInfo, type);
    
    QVERIFY(spy.count() == 1);
}

void TestKyWirelessConnectOperation::test_addTlsConnect_notFound()
{
    QSignalSpy spy(m_wirelessConnectOperation, &KyWirelessConnectOperation::createConnectionError);
    
    KyWirelessConnectSetting connSettingInfo;
    connSettingInfo.m_ssid = "TestSSID";
    connSettingInfo.m_ifaceName = "wlan0";
    KyEapMethodTlsInfo tlsInfo;
    
    m_wirelessConnectOperation->addTlsConnect(connSettingInfo, tlsInfo);
    
    QVERIFY(spy.count() == 1);
}

void TestKyWirelessConnectOperation::test_addPeapConnect_notFound()
{
    QSignalSpy spy(m_wirelessConnectOperation, &KyWirelessConnectOperation::createConnectionError);
    
    KyWirelessConnectSetting connSettingInfo;
    connSettingInfo.m_ssid = "TestSSID";
    connSettingInfo.m_ifaceName = "wlan0";
    KyEapMethodPeapInfo peapInfo;
    
    m_wirelessConnectOperation->addPeapConnect(connSettingInfo, peapInfo);
    
    QVERIFY(spy.count() == 1);
}

void TestKyWirelessConnectOperation::test_addTtlsConnect_notFound()
{
    QSignalSpy spy(m_wirelessConnectOperation, &KyWirelessConnectOperation::createConnectionError);
    
    KyWirelessConnectSetting connSettingInfo;
    connSettingInfo.m_ssid = "TestSSID";
    connSettingInfo.m_ifaceName = "wlan0";
    KyEapMethodTtlsInfo ttlsInfo;
    
    m_wirelessConnectOperation->addTtlsConnect(connSettingInfo, ttlsInfo);
    
    QVERIFY(spy.count() == 1);
}

void TestKyWirelessConnectOperation::test_addLeapConnect_notFound()
{
    QSignalSpy spy(m_wirelessConnectOperation, &KyWirelessConnectOperation::createConnectionError);
    
    KyWirelessConnectSetting connSettingInfo;
    connSettingInfo.m_ssid = "TestSSID";
    connSettingInfo.m_ifaceName = "wlan0";
    KyEapMethodLeapInfo leapInfo;
    
    m_wirelessConnectOperation->addLeapConnect(connSettingInfo, leapInfo);
    
    QVERIFY(spy.count() == 1);
}

void TestKyWirelessConnectOperation::test_addPwdConnect_notFound()
{
    QSignalSpy spy(m_wirelessConnectOperation, &KyWirelessConnectOperation::createConnectionError);
    
    KyWirelessConnectSetting connSettingInfo;
    connSettingInfo.m_ssid = "TestSSID";
    connSettingInfo.m_ifaceName = "wlan0";
    KyEapMethodPwdInfo pwdInfo;
    
    m_wirelessConnectOperation->addPwdConnect(connSettingInfo, pwdInfo);
    
    QVERIFY(spy.count() == 1);
}

void TestKyWirelessConnectOperation::test_addFastConnect_notFound()
{
    QSignalSpy spy(m_wirelessConnectOperation, &KyWirelessConnectOperation::createConnectionError);
    
    KyWirelessConnectSetting connSettingInfo;
    connSettingInfo.m_ssid = "TestSSID";
    connSettingInfo.m_ifaceName = "wlan0";
    KyEapMethodFastInfo fastInfo;
    
    m_wirelessConnectOperation->addFastConnect(connSettingInfo, fastInfo);
    
    QVERIFY(spy.count() == 1);
}

void TestKyWirelessConnectOperation::test_addAndActiveWirelessConnect_notFound()
{
    QSignalSpy spy(m_wirelessConnectOperation, &KyWirelessConnectOperation::activateConnectionError);
    
    KyWirelessConnectSetting connSettingInfo;
    connSettingInfo.m_ssid = "TestSSID";
    connSettingInfo.m_ifaceName = "wlan0";
    KySecuType type = NONE;
    
    m_wirelessConnectOperation->addAndActiveWirelessConnect(connSettingInfo, type);
    
    QVERIFY(spy.count() == 1);
}

void TestKyWirelessConnectOperation::test_addAndActiveWirelessEnterPriseTlsConnect_notFound()
{
    QSignalSpy spy(m_wirelessConnectOperation, &KyWirelessConnectOperation::activateConnectionError);
    
    KyWirelessConnectSetting connSettingInfo;
    connSettingInfo.m_ssid = "TestSSID";
    connSettingInfo.m_ifaceName = "wlan0";
    KyEapMethodTlsInfo tlsInfo;
    
    m_wirelessConnectOperation->addAndActiveWirelessEnterPriseTlsConnect(tlsInfo, connSettingInfo);
    
    QVERIFY(spy.count() == 1);
}

void TestKyWirelessConnectOperation::test_addAndActiveWirelessEnterPrisePeapConnect_notFound()
{
    QSignalSpy spy(m_wirelessConnectOperation, &KyWirelessConnectOperation::activateConnectionError);
    
    KyWirelessConnectSetting connSettingInfo;
    connSettingInfo.m_ssid = "TestSSID";
    connSettingInfo.m_ifaceName = "wlan0";
    KyEapMethodPeapInfo peapInfo;
    
    m_wirelessConnectOperation->addAndActiveWirelessEnterPrisePeapConnect(peapInfo, connSettingInfo);
    
    QVERIFY(spy.count() == 1);
}

void TestKyWirelessConnectOperation::test_addAndActiveWirelessEnterPriseTtlsConnect_notFound()
{
    QSignalSpy spy(m_wirelessConnectOperation, &KyWirelessConnectOperation::activateConnectionError);
    
    KyWirelessConnectSetting connSettingInfo;
    connSettingInfo.m_ssid = "TestSSID";
    connSettingInfo.m_ifaceName = "wlan0";
    KyEapMethodTtlsInfo ttlsInfo;
    
    m_wirelessConnectOperation->addAndActiveWirelessEnterPriseTtlsConnect(ttlsInfo, connSettingInfo);
    
    QVERIFY(spy.count() == 1);
}

void TestKyWirelessConnectOperation::test_addAndActiveWirelessEnterPriseLeapConnect_notFound()
{
    QSignalSpy spy(m_wirelessConnectOperation, &KyWirelessConnectOperation::activateConnectionError);
    
    KyWirelessConnectSetting connSettingInfo;
    connSettingInfo.m_ssid = "TestSSID";
    connSettingInfo.m_ifaceName = "wlan0";
    KyEapMethodLeapInfo leapInfo;
    
    m_wirelessConnectOperation->addAndActiveWirelessEnterPriseLeapConnect(leapInfo, connSettingInfo);
    
    QVERIFY(spy.count() == 1);
}

void TestKyWirelessConnectOperation::test_addAndActiveWirelessEnterPrisePwdConnect_notFound()
{
    QSignalSpy spy(m_wirelessConnectOperation, &KyWirelessConnectOperation::activateConnectionError);
    
    KyWirelessConnectSetting connSettingInfo;
    connSettingInfo.m_ssid = "TestSSID";
    connSettingInfo.m_ifaceName = "wlan0";
    KyEapMethodPwdInfo pwdInfo;
    
    m_wirelessConnectOperation->addAndActiveWirelessEnterPrisePwdConnect(pwdInfo, connSettingInfo);
    
    QVERIFY(spy.count() == 1);
}

void TestKyWirelessConnectOperation::test_addAndActiveWirelessEnterPriseFastConnect_notFound()
{
    QSignalSpy spy(m_wirelessConnectOperation, &KyWirelessConnectOperation::activateConnectionError);
    
    KyWirelessConnectSetting connSettingInfo;
    connSettingInfo.m_ssid = "TestSSID";
    connSettingInfo.m_ifaceName = "wlan0";
    KyEapMethodFastInfo fastInfo;
    
    m_wirelessConnectOperation->addAndActiveWirelessEnterPriseFastConnect(fastInfo, connSettingInfo);
    
    QVERIFY(spy.count() == 1);
}

void TestKyWirelessConnectOperation::test_activeWirelessAp_notFound()
{
    QString invalidUuid = "00000000-0000-0000-0000-000000000000";
    QString apName = "TestAP";
    QString apPassword = "password";
    QString apDevice = "wlan0";
    QString wirelessBand = "2.4Ghz";
    
    m_wirelessConnectOperation->activeWirelessAp(invalidUuid, apName, apPassword, apDevice, wirelessBand);
    QTest::qWait(100);
    QVERIFY(true);
}

void TestKyWirelessConnectOperation::test_deactiveWirelessAp_notFound()
{
    QString apName = "TestAP";
    QString apUuid = "00000000-0000-0000-0000-000000000000";
    
    m_wirelessConnectOperation->deactiveWirelessAp(apName, apUuid);
    QVERIFY(true);
}

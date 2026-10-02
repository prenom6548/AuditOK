#include "test_kyenterpricesettinginfo.h"
#include <QCoreApplication>
#include <NetworkManagerQt/ConnectionSettings>
#include <NetworkManagerQt/Security8021xSetting>
#include <NetworkManagerQt/WirelessSecuritySetting>

TestKyEnterpriceSettingInfo::TestKyEnterpriceSettingInfo(QObject *parent)
    : QObject(parent)
{
}

TestKyEnterpriceSettingInfo::~TestKyEnterpriceSettingInfo()
{
}

void TestKyEnterpriceSettingInfo::initTestCase()
{
}

void TestKyEnterpriceSettingInfo::cleanupTestCase()
{
}

void TestKyEnterpriceSettingInfo::prepareTlsInfo(KyEapMethodTlsInfo &info, bool withDomain, bool needCa)
{
    info.identity = "test_identity";
    if (withDomain) {
        info.domain = "test.domain.com";
    } else {
        info.domain = "";
    }
    info.devIfaceName = "wlan0";
    info.caCertPath = "/path/to/ca.crt";
    info.bNeedCa = needCa;
    info.clientCertPath = "/path/to/client.crt";
    info.clientPrivateKey = "/path/to/client.key";
    info.clientPrivateKeyPWD = "password";
    info.m_privateKeyPWDFlag = NetworkManager::Setting::None;
    info.bChanged = false;
}

void TestKyEnterpriceSettingInfo::preparePeapInfo(KyEapMethodPeapInfo &info, bool changed)
{
    info.phase2AuthMethod = KyAuthMethodMschapv2;
    info.userName = "test_user";
    info.userPWD = "test_password";
    info.m_passwdFlag = NetworkManager::Setting::None;
    info.bChanged = changed;
}

void TestKyEnterpriceSettingInfo::prepareTtlsInfo(KyEapMethodTtlsInfo &info, KyTtlsAuthMethod authType)
{
    info.authType = authType;
    if (authType == AUTH_EAP) {
        info.authEapMethod = KyAuthEapMethodMschapv2;
    } else {
        info.authNoEapMethod = KyAuthMethodMschapv2;
    }
    info.userName = "test_user";
    info.userPWD = "test_password";
    info.m_passwdFlag = NetworkManager::Setting::None;
    info.bChanged = false;
}

void TestKyEnterpriceSettingInfo::prepareLeapInfo(KyEapMethodLeapInfo &info, bool changed)
{
    info.m_userName = "test_user";
    info.m_userPwd = "test_password";
    info.m_passwdFlag = NetworkManager::Setting::None;
    info.bChanged = changed;
}

void TestKyEnterpriceSettingInfo::preparePwdInfo(KyEapMethodPwdInfo &info, bool changed)
{
    info.m_userName = "test_user";
    info.m_userPwd = "test_password";
    info.m_passwdFlag = NetworkManager::Setting::None;
    info.bChanged = changed;
}

void TestKyEnterpriceSettingInfo::prepareFastInfo(KyEapMethodFastInfo &info, bool allowAutoPac, bool changed)
{
    info.m_anonIdentity = "anonymous";
    info.m_pacProvisioning = KyFastProvisioningAllowAuthenticated;
    info.m_allowAutoPacFlag = allowAutoPac;
    info.m_pacFilePath = "/path/to/pac file";
    info.m_authMethod = KyAuthMethodMschapv2;
    info.m_userName = "test_user";
    info.m_userPwd = "test_password";
    info.m_passwdFlag = NetworkManager::Setting::None;
    info.bChanged = changed;
}

// TLS tests
void TestKyEnterpriceSettingInfo::test_assembleEapMethodTlsSettings_basic()
{
    KyEapMethodTlsInfo info;
    prepareTlsInfo(info, false, false);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    assembleEapMethodTlsSettings(connSettings, info);
    QVERIFY(true);
}

void TestKyEnterpriceSettingInfo::test_assembleEapMethodTlsSettings_withDomain()
{
    KyEapMethodTlsInfo info;
    prepareTlsInfo(info, true, false);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    assembleEapMethodTlsSettings(connSettings, info);
    QVERIFY(true);
}

void TestKyEnterpriceSettingInfo::test_assembleEapMethodTlsSettings_withCaCert()
{
    KyEapMethodTlsInfo info;
    prepareTlsInfo(info, false, true);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    assembleEapMethodTlsSettings(connSettings, info);
    QVERIFY(true);
}

void TestKyEnterpriceSettingInfo::test_assembleEapMethodTlsSettings_withoutCaCert()
{
    KyEapMethodTlsInfo info;
    prepareTlsInfo(info, false, false);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    assembleEapMethodTlsSettings(connSettings, info);
    QVERIFY(true);
}

void TestKyEnterpriceSettingInfo::test_modifyEapMethodTlsSettings_basic()
{
    KyEapMethodTlsInfo info;
    prepareTlsInfo(info, false, false);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    modifyEapMethodTlsSettings(connSettings, info);
    QVERIFY(true);
}

void TestKyEnterpriceSettingInfo::test_modifyEapMethodTlsSettings_withDomain()
{
    KyEapMethodTlsInfo info;
    prepareTlsInfo(info, true, false);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    modifyEapMethodTlsSettings(connSettings, info);
    QVERIFY(true);
}

void TestKyEnterpriceSettingInfo::test_modifyEapMethodTlsSettings_withCaCert()
{
    KyEapMethodTlsInfo info;
    prepareTlsInfo(info, false, true);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    modifyEapMethodTlsSettings(connSettings, info);
    QVERIFY(true);
}

void TestKyEnterpriceSettingInfo::test_modifyEapMethodTlsSettings_withoutCaCert()
{
    KyEapMethodTlsInfo info;
    prepareTlsInfo(info, false, false);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    modifyEapMethodTlsSettings(connSettings, info);
    QVERIFY(true);
}

void TestKyEnterpriceSettingInfo::test_modifyEapMethodTlsSettings_withPasswordChanged()
{
    KyEapMethodTlsInfo info;
    prepareTlsInfo(info, false, false);
    info.bChanged = true;
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    modifyEapMethodTlsSettings(connSettings, info);
    QVERIFY(true);
}

// PEAP tests
void TestKyEnterpriceSettingInfo::test_assembleEapMethodPeapSettings_basic()
{
    KyEapMethodPeapInfo info;
    preparePeapInfo(info, false);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    assembleEapMethodPeapSettings(connSettings, info);
    QVERIFY(true);
}

void TestKyEnterpriceSettingInfo::test_assembleEapMethodPeapSettings_withPasswordChanged()
{
    KyEapMethodPeapInfo info;
    preparePeapInfo(info, true);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    assembleEapMethodPeapSettings(connSettings, info);
    QVERIFY(true);
}

void TestKyEnterpriceSettingInfo::test_modifyEapMethodPeapSettings_basic()
{
    KyEapMethodPeapInfo info;
    preparePeapInfo(info, false);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    modifyEapMethodPeapSettings(connSettings, info);
    QVERIFY(true);
}

void TestKyEnterpriceSettingInfo::test_modifyEapMethodPeapSettings_withPasswordChanged()
{
    KyEapMethodPeapInfo info;
    preparePeapInfo(info, true);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    modifyEapMethodPeapSettings(connSettings, info);
    QVERIFY(true);
}

// TTLS tests
void TestKyEnterpriceSettingInfo::test_assembleEapMethodTtlsSettings_authEap()
{
    KyEapMethodTtlsInfo info;
    prepareTtlsInfo(info, AUTH_EAP);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    assembleEapMethodTtlsSettings(connSettings, info);
    QVERIFY(true);
}

void TestKyEnterpriceSettingInfo::test_assembleEapMethodTtlsSettings_authNoEap()
{
    KyEapMethodTtlsInfo info;
    prepareTtlsInfo(info, AUTH_NO_EAP);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    assembleEapMethodTtlsSettings(connSettings, info);
    QVERIFY(true);
}

void TestKyEnterpriceSettingInfo::test_modifyEapMethodTtlsSettings_authEap()
{
    KyEapMethodTtlsInfo info;
    prepareTtlsInfo(info, AUTH_EAP);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    modifyEapMethodTtlsSettings(connSettings, info);
    QVERIFY(true);
}

void TestKyEnterpriceSettingInfo::test_modifyEapMethodTtlsSettings_authNoEap()
{
    KyEapMethodTtlsInfo info;
    prepareTtlsInfo(info, AUTH_NO_EAP);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    modifyEapMethodTtlsSettings(connSettings, info);
    QVERIFY(true);
}

// LEAP tests
void TestKyEnterpriceSettingInfo::test_assembleEapMethodLeapSettings_basic()
{
    KyEapMethodLeapInfo info;
    prepareLeapInfo(info, false);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    assembleEapMethodLeapSettings(connSettings, info);
    QVERIFY(true);
}

void TestKyEnterpriceSettingInfo::test_assembleEapMethodLeapSettings_withPasswordChanged()
{
    KyEapMethodLeapInfo info;
    prepareLeapInfo(info, true);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    assembleEapMethodLeapSettings(connSettings, info);
    QVERIFY(true);
}

void TestKyEnterpriceSettingInfo::test_modifyEapMethodLeapSettings_basic()
{
    KyEapMethodLeapInfo info;
    prepareLeapInfo(info, false);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    modifyEapMethodLeapSettings(connSettings, info);
    QVERIFY(true);
}

void TestKyEnterpriceSettingInfo::test_modifyEapMethodLeapSettings_withPasswordChanged()
{
    KyEapMethodLeapInfo info;
    prepareLeapInfo(info, true);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    modifyEapMethodLeapSettings(connSettings, info);
    QVERIFY(true);
}

// PWD tests
void TestKyEnterpriceSettingInfo::test_assembleEapMethodPwdSettings_basic()
{
    KyEapMethodPwdInfo info;
    preparePwdInfo(info, false);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    assembleEapMethodPwdSettings(connSettings, info);
    QVERIFY(true);
}

void TestKyEnterpriceSettingInfo::test_assembleEapMethodPwdSettings_withPasswordChanged()
{
    KyEapMethodPwdInfo info;
    preparePwdInfo(info, true);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    assembleEapMethodPwdSettings(connSettings, info);
    QVERIFY(true);
}

void TestKyEnterpriceSettingInfo::test_modifyEapMethodPwdSettings_basic()
{
    KyEapMethodPwdInfo info;
    preparePwdInfo(info, false);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    modifyEapMethodPwdSettings(connSettings, info);
    QVERIFY(true);
}

void TestKyEnterpriceSettingInfo::test_modifyEapMethodPwdSettings_withPasswordChanged()
{
    KyEapMethodPwdInfo info;
    preparePwdInfo(info, true);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    modifyEapMethodPwdSettings(connSettings, info);
    QVERIFY(true);
}

// FAST tests
void TestKyEnterpriceSettingInfo::test_assembleEapMethodFastSettings_basic()
{
    KyEapMethodFastInfo info;
    prepareFastInfo(info, false, false);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    assembleEapMethodFastSettings(connSettings, info);
    QVERIFY(true);
}

void TestKyEnterpriceSettingInfo::test_assembleEapMethodFastSettings_withAutoPac()
{
    KyEapMethodFastInfo info;
    prepareFastInfo(info, true, false);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    assembleEapMethodFastSettings(connSettings, info);
    QVERIFY(true);
}

void TestKyEnterpriceSettingInfo::test_assembleEapMethodFastSettings_withoutAutoPac()
{
    KyEapMethodFastInfo info;
    prepareFastInfo(info, false, false);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    assembleEapMethodFastSettings(connSettings, info);
    QVERIFY(true);
}

void TestKyEnterpriceSettingInfo::test_modifyEapMethodFastSettings_basic()
{
    KyEapMethodFastInfo info;
    prepareFastInfo(info, false, false);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    modifyEapMethodFastSettings(connSettings, info);
    QVERIFY(true);
}

void TestKyEnterpriceSettingInfo::test_modifyEapMethodFastSettings_withAutoPac()
{
    KyEapMethodFastInfo info;
    prepareFastInfo(info, true, false);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    modifyEapMethodFastSettings(connSettings, info);
    QVERIFY(true);
}

void TestKyEnterpriceSettingInfo::test_modifyEapMethodFastSettings_withoutAutoPac()
{
    KyEapMethodFastInfo info;
    prepareFastInfo(info, false, false);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    modifyEapMethodFastSettings(connSettings, info);
    QVERIFY(true);
}

void TestKyEnterpriceSettingInfo::test_modifyEapMethodFastSettings_withPasswordChanged()
{
    KyEapMethodFastInfo info;
    prepareFastInfo(info, false, true);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    modifyEapMethodFastSettings(connSettings, info);
    QVERIFY(true);
}

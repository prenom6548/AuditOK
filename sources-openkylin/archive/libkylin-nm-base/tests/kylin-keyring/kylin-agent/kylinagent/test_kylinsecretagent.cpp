#include "test_kylinsecretagent.h"
#include <QCoreApplication>
#include <QDebug>
#include <QTest>
#include <QWidget>
TestKylinSecretAgent::TestKylinSecretAgent(QObject *parent)
    : QObject(parent)
    , m_agent(nullptr)
{
}

TestKylinSecretAgent::~TestKylinSecretAgent()
{
}


void TestKylinSecretAgent::SetUp()
{
    QWidget* pWidget=new QWidget();
    m_agent = new KylinSecretAgent("test-agent", pWidget, nullptr);
    m_agent->setTestMod(1);
}

void TestKylinSecretAgent::TearDown()
{
    if (m_agent) {
        delete m_agent;
        m_agent = nullptr;
    }
}



void TestKylinSecretAgent::GetSecretsWithEmptyConnection()
{
    NMVariantMapMap connection;
    QDBusObjectPath path("/org/freedesktop/NetworkManager/Settings/1");
    QString settingName = "802-11-wireless-security";
    QStringList hints;
    uint flags = 0;

    NMVariantMapMap result = m_agent->GetSecrets(connection, path, settingName, hints, flags);

    m_agent->connectionIsAlwaysAsk(connection);
    //EXPECT_TRUE(result.isEmpty());
}

void TestKylinSecretAgent::GetSecretsWithVpnConnection()
{
    NMVariantMapMap connection;

    QVariantMap connMap;
    connMap["id"] = "test-vpn";
    connMap["uuid"] = "test-uuid-1234";
    connMap["type"] = "vpn";
    connection["connection"] = connMap;

    QVariantMap vpnMap;
    vpnMap["service-type"] = "org.freedesktop.NetworkManager.pptp";
    connection["vpn"] = vpnMap;

    QDBusObjectPath path("/org/freedesktop/NetworkManager/Settings/1");
    QString settingName = "vpn";
    QStringList hints;
    uint flags = 0;

    NMVariantMapMap result = m_agent->GetSecrets(connection, path, settingName, hints, flags);
}

void TestKylinSecretAgent::CancelGetSecretsTest()
{
    QDBusObjectPath path("/org/freedesktop/NetworkManager/Settings/1");
    QString settingName = "vpn";

    m_agent->CancelGetSecrets(path, settingName);
}

void TestKylinSecretAgent::SaveSecretsTest()
{
    NMVariantMapMap connection;

    QVariantMap connMap;
    QVariantMap wirelessConnMap;
    connMap["id"] = "test-connection";
    connMap["uuid"] = "test-uuid-1234";
    connMap[TYPE_KEY] = "802-11-wireless";
    connection[CONNECTION_KEY] = connMap;

    QVariantMap wirelessMap;
    wirelessMap["ssid"] = QByteArray("TestAP");
    wirelessMap[PSK_FLAG] = POLICY_AGENT_OWNED;
    wirelessMap[PSK_KEY] = "ADBCDEF123456";

    //wirelessMap[LEAP_PASSWORD_FLAG] = LEAP_PASSWORD_KEY;
    connection[WIRELESS_SECURITY_NAME] = wirelessMap;
    QDBusObjectPath path("/org/freedesktop/NetworkManager/Settings/1");
    m_agent->SaveSecrets(connection, path);
    connection.clear();
   
   QVariantMap vpnMap;
    connMap[TYPE_KEY] = VPN_TYPE;
    connMap["uuid"] = "test-uuid-5678";
    vpnMap[VPN_DATA]="i,am,you,password-flags,her";
    vpnMap[VPN_SECRET]="key=password123456789012345";

    connection[CONNECTION_KEY] = connMap;
    connection[VPN_NAME] = vpnMap;
    m_agent->SaveSecrets(connection, path);
    connection.clear();

/*测试企业wifi*/
    QVariantMap enWirelessMap;
    connMap[TYPE_KEY] = ENTERPRICE_NAME;
    connMap["uuid"] = "test-uuid-9999";
    enWirelessMap["ssid"] = QByteArray("TestAP");
    enWirelessMap[PASSWORD_FLAG] = POLICY_AGENT_OWNED;
    enWirelessMap[PASSWORD_KEY] = "ADBCDEF123456";
    connection[CONNECTION_KEY] = connMap;
    connection[ENTERPRICE_NAME] = enWirelessMap;
    m_agent->SaveSecrets(connection, path);
    connection.clear();
   
  

       /*测试未知*/
    QVariantMap unkownMap;
    connMap[TYPE_KEY] = "unkown";
    connMap["uuid"] = "test-uuid-11111";
    connection[CONNECTION_KEY] = connMap;
    connection["unkown"] = unkownMap;
    m_agent->SaveSecrets(connection, path);
}

void TestKylinSecretAgent::DeleteSecretsTest()
{
    NMVariantMapMap connection;

    QVariantMap connMap;
    connMap["id"] = "test-connection";
    connMap["uuid"] = "test-uuid-1234";
    connMap["type"] = "802-11-wireless";
    connection["connection"] = connMap;

    QDBusObjectPath path("/org/freedesktop/NetworkManager/Settings/1");

    m_agent->DeleteSecrets(connection, path);
}

void TestKylinSecretAgent::GetSecretsWithWirelessConnection()
{
    NMVariantMapMap connection;

    QVariantMap connMap;
    connMap["id"] = "test-wifi";
    connMap["uuid"] = "test-uuid-5678";
    connMap["type"] = "802-11-wireless";
    connection["connection"] = connMap;

    QVariantMap wirelessMap;
    wirelessMap["ssid"] = QByteArray("TestAP");
    connection["802-11-wireless"] = wirelessMap;

    QVariantMap securityMap;
    securityMap["key-mgmt"] = "wpa-psk";
    securityMap["psk"] = "test-password";
    connection["802-11-wireless-security"] = securityMap;

    QDBusObjectPath path("/org/freedesktop/NetworkManager/Settings/2");
    QString settingName = "802-11-wireless-security";
    QStringList hints;
    hints.append("psk");
    uint flags = 0;

    NMVariantMapMap result = m_agent->GetSecrets(connection, path, settingName, hints, flags);
}

void TestKylinSecretAgent::GetSecretsWithEnterpriseConnection()
{
    NMVariantMapMap connection;

    QVariantMap connMap;
    connMap["id"] = "test-enterprise";
    connMap["uuid"] = "test-uuid-9999";
    connMap["type"] = "802-11-wireless";
    connection["connection"] = connMap;

    QVariantMap wirelessMap;
    wirelessMap["ssid"] = QByteArray("EnterpriseAP");
    connection["802-11-wireless"] = wirelessMap;

    QVariantMap securityMap;
    securityMap["key-mgmt"] = "wpa-eap";
    securityMap["eap"] = "ttls";
    securityMap["identity"] = "test-user";
    connection["802-11-wireless-security"] = securityMap;

    QVariantMap enterpriseMap;
    enterpriseMap["eap"] = "ttls";
    enterpriseMap["identity"] = "test-user";
    enterpriseMap["password"] = "test-password";
    connection["802-1x"] = enterpriseMap;

    QDBusObjectPath path("/org/freedesktop/NetworkManager/Settings/3");
    QString settingName = "802-1x";
    QStringList hints;
    hints.append("password");
    uint flags = 0;

    NMVariantMapMap result = m_agent->GetSecrets(connection, path, settingName, hints, flags);
}

void TestKylinSecretAgent::AskSecretForPasswordTest()
{
    QVariantMap securityMap;
    QVariantMap secretKeyMap;
    
    securityMap["psk"] = "initial_password";
    
    QString passwordKey = "psk";
    QString settingName = "802-11-wireless-security";
    NMVariantMapMap secretMap;
    QString connectionID = "test_connection";
    
    // Initially, secretMap should be empty
    QVERIFY(secretMap.isEmpty());
    
    // Call the function
    m_agent->askSecretForPassword(securityMap, passwordKey, settingName, secretMap, connectionID);

    secretKeyMap[passwordKey]="12345678";
    secretMap[settingName]=secretKeyMap;
    m_agent->askSecretForPassword(securityMap, passwordKey, settingName, secretMap, connectionID);
    // Since we are in test mode, the dialog won't actually pop up,
    // but we can still verify the behavior based on the implementation
    // In test mode, the function should not modify secretMap when dialog is cancelled
    //QVERIFY(secretMap.isEmpty());
}

void TestKylinSecretAgent::AskSecretForWepTest()
{
    QVariantMap wirelessSecurityMap;
    QVariantMap secretKeyMap;
    wirelessSecurityMap["wep-tx-keyidx"] = KEY_TYPE0;
    wirelessSecurityMap["wep-key0"] = "initial_wep_key";
    
    secretKeyMap[WEP_KEY0]="12345678";
    QString settingName = "802-11-wireless-security";
    NMVariantMapMap secretMap;
    QString connectionID = "test_connection";
    
    QVERIFY(secretMap.isEmpty());
    
    m_agent->askSecretForWep(wirelessSecurityMap, settingName, secretMap, connectionID);
    
    secretMap["802-11-wireless-security"]=secretKeyMap;
    m_agent->askSecretForWep(wirelessSecurityMap, settingName, secretMap, connectionID);

    wirelessSecurityMap["wep-tx-keyidx"] = 255;
    m_agent->askSecretForWep(wirelessSecurityMap, settingName, secretMap, connectionID);
    //QVERIFY(secretMap.isEmpty());
}

void TestKylinSecretAgent::AskSecretForLeapTest()
{
    QVariantMap wirelessSecurityMap;
    QVariantMap secretKeyMap;
    wirelessSecurityMap["leap-username"] = "initial_user";
    wirelessSecurityMap["leap-password"] = "initial_password";
    
    secretKeyMap[LEAP_PASSWORD_KEY]="12345678";
    QString settingName = "802-11-wireless-security";
    NMVariantMapMap secretMap;
    QString connectionID = "test_connection";
    
    QVERIFY(secretMap.isEmpty());
    m_agent->askSecretForLeap(wirelessSecurityMap, settingName, secretMap, connectionID);

    secretMap["802-11-wireless-security"]=secretKeyMap;
    m_agent->askSecretForLeap(wirelessSecurityMap, settingName, secretMap, connectionID);
    
    //QVERIFY(secretMap.isEmpty());
}

void TestKylinSecretAgent::AskSecretForWirelessTest()
{
    NMVariantMapMap connection;
    
    QVariantMap connMap;
    connMap["id"] = "test-wifi";
    connMap["uuid"] = "test-uuid-5678";
    connection["connection"] = connMap;
    
    QVariantMap securityMap;

    connection["802-11-wireless-security"] = securityMap;
    
    QString settingName = "802-11-wireless-security";
    NMVariantMapMap secretMap;
    
    QVERIFY(secretMap.isEmpty());
    m_agent->askSecretForWireless(connection, settingName, secretMap);
    
    securityMap["key-mgmt"] = "wpa-psk";
    securityMap["psk"] = "test-password";
    m_agent->askSecretForWireless(connection, settingName, secretMap);

    securityMap["key-mgmt"] = "unkknown-key-mgmt";
    connection["802-11-wireless-security"] = securityMap;
    m_agent->askSecretForWireless(connection, settingName, secretMap);
    //QVERIFY(secretMap.isEmpty());
}

void TestKylinSecretAgent::AskSecretWithIdentityAndPasswordTest()
{
    QVariantMap securityMap;
    securityMap["identity"] = "test_user";
    securityMap["password"] = "initial_password";
    
    QString userKey = "identity";
    QString passwordKey = "password";
    QString settingName = "802-1x";
    NMVariantMapMap secretMap;
    QString connectionID = "test_connection";
    
    QVERIFY(secretMap.isEmpty());
    
    m_agent->askSecretWithIdentityAndPassword(securityMap, userKey, passwordKey, settingName, secretMap, connectionID);
    
    QVariantMap secretKeyMap;
    secretKeyMap[passwordKey]="12345678";
    secretMap[settingName]=secretKeyMap;
    m_agent->askSecretWithIdentityAndPassword(securityMap, userKey, passwordKey, settingName, secretMap, connectionID);

    //QVERIFY(secretMap.isEmpty());
}

void TestKylinSecretAgent::AskSecretForTlsTest()
{
    QVariantMap securityMap;
    QVariantMap secretKeyMap;
    secretKeyMap[PRIVATE_KEY] = "12345678";
    securityMap["identity"] = "test_user";
    securityMap["private-key-password"] = "initial_password";
    securityMap[DOMAIN_KEY]="baidu";
    
    QString settingName = "802-1x";
    NMVariantMapMap secretMap;
    QString connectionID = "test_connection";
    
    QVERIFY(secretMap.isEmpty());
    
    m_agent->askSecretForTls(securityMap, settingName, secretMap, connectionID);

    secretMap["802-11-wireless-security"]=secretKeyMap;
    m_agent->askSecretForTls(securityMap, settingName, secretMap, connectionID);
    
    //QVERIFY(secretMap.isEmpty());
}

void TestKylinSecretAgent::AskSecretForEnterpriceWirelessTest()
{
    NMVariantMapMap connection;
    
    QVariantMap connMap;
    connMap["id"] = "test-enterprise";
    connMap["uuid"] = "test-uuid-9999";
    connection["connection"] = connMap;
    
    QVariantMap securityMap;

    connection["802-1x"] = securityMap;
    
    QString settingName = "802-1x";
    NMVariantMapMap secretMap;
    
    QVERIFY(secretMap.isEmpty());
    
    m_agent->askSecretForEnterpriceWireless(connection, settingName, secretMap);
    

    securityMap["eap"] = "ttls";
    securityMap["identity"] = "test_user";
    securityMap["password"] = "initial_password";
    m_agent->askSecretForEnterpriceWireless(connection, settingName, secretMap);
    //QVERIFY(secretMap.isEmpty());
}

void TestKylinSecretAgent::AskSecretForVpnTest()
{
    NMVariantMapMap connection;
    
    QVariantMap connMap;
    connMap["id"] = "test-vpn";
    connMap["uuid"] = "test-uuid-1234";
    connection["connection"] = connMap;
    
    QVariantMap vpnMap;

    connection["vpn"] = vpnMap;
    
    QString settingName = "vpn";
    NMVariantMapMap secretMap;
    
    QVERIFY(secretMap.isEmpty());
    
    m_agent->askSecretForVpn(connection, settingName, secretMap);
    
    vpnMap["service-type"] = "org.freedesktop.NetworkManager.pptp";
    vpnMap["secrets"] = "password=initial_password";
    m_agent->askSecretForVpn(connection, settingName, secretMap);
    //QVERIFY(secretMap.isEmpty());
}

void TestKylinSecretAgent::AskForSecretTest()
{
    NMVariantMapMap connection;
    
    QVariantMap connMap;
    connMap["id"] = "test-wifi";
    connMap["uuid"] = "test-uuid-5678";
    connMap["type"] = "802-11-wireless";
    connection["connection"] = connMap;
    
    QVariantMap securityMap;
    securityMap["key-mgmt"] = "wpa-psk";
    securityMap["psk"] = "test-password";
    connection["802-11-wireless-security"] = securityMap;
    
    QString settingName = "802-11-wireless-security";
    QString connectType = "802-11-wireless";
    NMVariantMapMap secretMap;
    
    QVERIFY(secretMap.isEmpty());
    
    m_agent->askForSecret(connection, settingName, connectType, secretMap);
    
    //QVERIFY(secretMap.isEmpty());
}

void TestKylinSecretAgent::GetWepKeyFromIdTest()
{
    m_agent->getWepKeyFromId(KEY_TYPE1);
    m_agent->getWepKeyFromId(KEY_TYPE2);
    m_agent->getWepKeyFromId(KEY_TYPE3);
    m_agent->getWepKeyFromId(255);
}

void TestKylinSecretAgent::SaveOneSecretTest()
{
    QString connectUuid;
    QString settingName;
    QString secretKey;
    QString secretValue;
    QString displayName;
     m_agent->saveOneSecret(connectUuid,settingName, secretKey, secretValue, displayName);

    QVariantMap securityMap;
    QString connectId;
    settingName="802-11-wireless-security";
    m_agent->saveConnectSecret(securityMap,settingName,secretKey,connectId,connectUuid);


    QVariantMap wirelessSecurityMap;
    wirelessSecurityMap[WEP_ID_KEY] = 255;
    m_agent->saveWepSecret(wirelessSecurityMap,connectId,connectUuid);

    wirelessSecurityMap[WEP_ID_KEY] = 0;
    m_agent->saveWepSecret(wirelessSecurityMap,connectId,connectUuid);

    wirelessSecurityMap[WEP_KEY0] = "";
    m_agent->saveWepSecret(wirelessSecurityMap,connectId,connectUuid);

    wirelessSecurityMap[WEP_KEY0] = "12345";
    m_agent->saveWepSecret(wirelessSecurityMap,connectId,connectUuid);

    QVariantMap vpnSetingMap;
    m_agent->saveSecretForVpn(vpnSetingMap,connectId,connectUuid);

    QString vpnData;


    vpnData=VPN_FLAG+QString("=")+QString(POLICY_AGENT_OWNED)+",user = admin";
    vpnSetingMap[VPN_DATA]=vpnData;
    m_agent->saveSecretForVpn(vpnSetingMap,connectId,connectUuid);

    vpnSetingMap[VPN_DATA]=VPN_FLAG;
    vpnSetingMap[VPN_SECRET]="TEST=12345678";
    m_agent->saveSecretForVpn(vpnSetingMap,connectId,connectUuid);


    wirelessSecurityMap.clear();
    wirelessSecurityMap[WEP_FLAG_KEY] = POLICY_AGENT_OWNED;
    m_agent->saveWirelessSecret(wirelessSecurityMap,connectId,connectUuid);

}

#include "test_kylinsecretagent.h"
#include <QCoreApplication>
#include <QDebug>

void TestKylinSecretAgent::SetUp()
{
    m_agent = new KylinSecretAgent("test-agent", nullptr, nullptr);
}

void TestKylinSecretAgent::TearDown()
{
    if (m_agent) {
        delete m_agent;
        m_agent = nullptr;
    }
}

void TestKylinSecretAgentFunctions::SetUp()
{
    m_agent = new KylinSecretAgent("test-agent", nullptr, nullptr);
}

void TestKylinSecretAgentFunctions::TearDown()
{
    if (m_agent) {
        delete m_agent;
        m_agent = nullptr;
    }
}

TEST_F(TestKylinSecretAgent, ConstructorTest)
{
    ASSERT_NE(m_agent, nullptr);
}

TEST_F(TestKylinSecretAgentFunctions, GetSecretsWithEmptyConnection)
{
    NMVariantMapMap connection;
    QDBusObjectPath path("/org/freedesktop/NetworkManager/Settings/1");
    QString settingName = "802-11-wireless-security";
    QStringList hints;
    uint flags = 0;

    NMVariantMapMap result = m_agent->GetSecrets(connection, path, settingName, hints, flags);
    //EXPECT_TRUE(result.isEmpty());
}

TEST_F(TestKylinSecretAgentFunctions, GetSecretsWithVpnConnection)
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

TEST_F(TestKylinSecretAgentFunctions, CancelGetSecretsTest)
{
    QDBusObjectPath path("/org/freedesktop/NetworkManager/Settings/1");
    QString settingName = "vpn";

    m_agent->CancelGetSecrets(path, settingName);
}

TEST_F(TestKylinSecretAgentFunctions, SaveSecretsTest)
{
    NMVariantMapMap connection;

    QVariantMap connMap;
    connMap["id"] = "test-connection";
    connMap["uuid"] = "test-uuid-1234";
    connMap["type"] = "802-11-wireless";
    connection["connection"] = connMap;

    QVariantMap wirelessMap;
    wirelessMap["ssid"] = QByteArray("TestAP");
    connection["802-11-wireless"] = wirelessMap;

    QDBusObjectPath path("/org/freedesktop/NetworkManager/Settings/1");

    //m_agent->SaveSecrets(connection, path);//暂时注释不测
}

TEST_F(TestKylinSecretAgentFunctions, DeleteSecretsTest)
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

TEST_F(TestKylinSecretAgentFunctions, GetSecretsWithWirelessConnection)
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

TEST_F(TestKylinSecretAgentFunctions, GetSecretsWithEnterpriseConnection)
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
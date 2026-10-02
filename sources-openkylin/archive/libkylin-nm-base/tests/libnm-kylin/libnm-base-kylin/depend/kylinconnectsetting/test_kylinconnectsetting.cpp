#include "test_kylinconnectsetting.h"
#include <QSignalSpy>
#include <QTest>
#include <QDebug>

TestKylinConnectSetting::TestKylinConnectSetting(QObject *parent)
    : QObject(parent)
{
}

TestKylinConnectSetting::~TestKylinConnectSetting()
{
}

void TestKylinConnectSetting::initTestCase()
{
    m_connectSetting = new KyConnectSetting();
}

void TestKylinConnectSetting::cleanupTestCase()
{
    if (m_connectSetting) {
        delete m_connectSetting;
        m_connectSetting = nullptr;
    }
}

void TestKylinConnectSetting::test_constructor()
{
    QVERIFY2(m_connectSetting != nullptr, "Failed to create KyConnectSetting instance");
    QVERIFY2(m_connectSetting->m_connectName == "", "m_connectName should be empty string");
    QVERIFY2(m_connectSetting->m_ifaceName == "", "m_ifaceName should be empty string");
    QVERIFY2(m_connectSetting->m_ipv4ConfigIpType == CONFIG_IP_DHCP, "m_ipv4ConfigIpType should be CONFIG_IP_DHCP");
    QVERIFY2(m_connectSetting->m_ipv6ConfigIpType == CONFIG_IP_DHCP, "m_ipv6ConfigIpType should be CONFIG_IP_DHCP");
    QVERIFY2(m_connectSetting->m_ipv4Address.isEmpty(), "m_ipv4Address should be empty");
    QVERIFY2(m_connectSetting->m_ipv4Dns.isEmpty(), "m_ipv4Dns should be empty");
    QVERIFY2(m_connectSetting->m_ipv6Address.isEmpty(), "m_ipv6Address should be empty");
    QVERIFY2(m_connectSetting->m_ipv6Dns.isEmpty(), "m_ipv6Dns should be empty");
    QVERIFY2(m_connectSetting->m_isAutoConnect == true, "m_isAutoConnect should be true");
}

void TestKylinConnectSetting::test_destructor()
{
    KyConnectSetting *temp = new KyConnectSetting();
    QVERIFY2(temp != nullptr, "Failed to create temporary KyConnectSetting instance");
    delete temp;
    temp = nullptr;
    QVERIFY2(temp == nullptr, "Destructor test: temp should be nullptr after delete");
}

void TestKylinConnectSetting::test_setIfaceName()
{
    QString ifaceName = "eth0";
    m_connectSetting->setIfaceName(ifaceName);
    QVERIFY2(m_connectSetting->m_ifaceName == "eth0", "m_ifaceName should be set to eth0");
}

void TestKylinConnectSetting::test_setConnectName()
{
    QString connectName = "Wired connection 1";
    m_connectSetting->setConnectName(connectName);
    QVERIFY2(m_connectSetting->m_connectName == "Wired connection 1", "m_connectName should be set to Wired connection 1");
}

void TestKylinConnectSetting::test_setIpConfigType_valid_v4()
{
    int result = m_connectSetting->setIpConfigType(IPADDRESS_V4, CONFIG_IP_DHCP);
    QVERIFY2(result == 0, "setIpConfigType should return 0 for valid v4 DHCP");
    QVERIFY2(m_connectSetting->m_ipv4ConfigIpType == CONFIG_IP_DHCP, "m_ipv4ConfigIpType should be CONFIG_IP_DHCP");

    result = m_connectSetting->setIpConfigType(IPADDRESS_V4, CONFIG_IP_MANUAL);
    QVERIFY2(result == 0, "setIpConfigType should return 0 for valid v4 MANUAL");
    QVERIFY2(m_connectSetting->m_ipv4ConfigIpType == CONFIG_IP_MANUAL, "m_ipv4ConfigIpType should be CONFIG_IP_MANUAL");
}

void TestKylinConnectSetting::test_setIpConfigType_valid_v6()
{
    int result = m_connectSetting->setIpConfigType(IPADDRESS_V6, CONFIG_IP_DHCP);
    QVERIFY2(result == 0, "setIpConfigType should return 0 for valid v6 DHCP");
    QVERIFY2(m_connectSetting->m_ipv6ConfigIpType == CONFIG_IP_DHCP, "m_ipv6ConfigIpType should be CONFIG_IP_DHCP");

    result = m_connectSetting->setIpConfigType(IPADDRESS_V6, CONFIG_IP_MANUAL);
    QVERIFY2(result == 0, "setIpConfigType should return 0 for valid v6 MANUAL");
    QVERIFY2(m_connectSetting->m_ipv6ConfigIpType == CONFIG_IP_MANUAL, "m_ipv6ConfigIpType should be CONFIG_IP_MANUAL");
}

void TestKylinConnectSetting::test_setIpConfigType_invalid_ipType()
{
    KyIpAddressType invalidIpType = static_cast<KyIpAddressType>(100);
    int result = m_connectSetting->setIpConfigType(invalidIpType, CONFIG_IP_DHCP);
    QVERIFY2(result == -EINVAL, "setIpConfigType should return -EINVAL for invalid ipType");
}

void TestKylinConnectSetting::test_setIpConfigType_invalid_configType()
{
    KyIpConfigType invalidConfigType = static_cast<KyIpConfigType>(100);
    int result = m_connectSetting->setIpConfigType(IPADDRESS_V4, invalidConfigType);
    QVERIFY2(result == -EINVAL, "setIpConfigType should return -EINVAL for invalid configType");
}

void TestKylinConnectSetting::test_ipv4AddressConstruct()
{
    QString ipv4Address = "192.168.1.100";
    QString ipv4NetMask = "255.255.255.0";
    QString ipv4GateWay = "192.168.1.1";
    QStringList ipv4Dns;
    ipv4Dns << "8.8.8.8" << "8.8.4.4";

    m_connectSetting->ipv4AddressConstruct(ipv4Address, ipv4NetMask, ipv4GateWay, ipv4Dns);

    QVERIFY2(m_connectSetting->m_ipv4Address.size() == 1, "m_ipv4Address should have 1 element");
    QVERIFY2(m_connectSetting->m_ipv4Address.at(0).ip().toString() == "192.168.1.100", "IPv4 address should match");
    QVERIFY2(m_connectSetting->m_ipv4Address.at(0).gateway().toString() == "192.168.1.1", "IPv4 gateway should match");
    QVERIFY2(m_connectSetting->m_ipv4Address.at(0).netmask().toString() == "255.255.255.0", "IPv4 netmask should match");

    QVERIFY2(m_connectSetting->m_ipv4Dns.size() == 2, "m_ipv4Dns should have 2 elements");
    QVERIFY2(m_connectSetting->m_ipv4Dns.at(0).toString() == "8.8.8.8", "First DNS should match");
    QVERIFY2(m_connectSetting->m_ipv4Dns.at(1).toString() == "8.8.4.4", "Second DNS should match");
}

void TestKylinConnectSetting::test_ipv6AddressConstruct()
{
    QString ipv6Address = "2001:db8::1";
    QString ipv6NetMask = "64";
    QString ipv6GateWay = "2001:db8::ffff";
    QStringList ipv6Dns;
    ipv6Dns << "2001:4860:4860::8888" << "2001:4860:4860::8844";

    m_connectSetting->ipv6AddressConstruct(ipv6Address, ipv6NetMask, ipv6GateWay, ipv6Dns);

    QVERIFY2(m_connectSetting->m_ipv6Address.size() == 1, "m_ipv6Address should have 1 element");
    QVERIFY2(m_connectSetting->m_ipv6Address.at(0).ip().toString() == "2001:db8::1", "IPv6 address should match");
    QVERIFY2(m_connectSetting->m_ipv6Address.at(0).gateway().toString() == "2001:db8::ffff", "IPv6 gateway should match");

    QVERIFY2(m_connectSetting->m_ipv6Dns.size() == 2, "m_ipv6Dns should have 2 elements");
    QVERIFY2(m_connectSetting->m_ipv6Dns.at(0).toString() == "2001:4860:4860::8888", "First IPv6 DNS should match");
    QVERIFY2(m_connectSetting->m_ipv6Dns.at(1).toString() == "2001:4860:4860::8844", "Second IPv6 DNS should match");
}

void TestKylinConnectSetting::test_dumpInfo()
{
    QString connName = "TestConnection";
    QString iface = "eth0";
    m_connectSetting->setConnectName(connName);
    m_connectSetting->setIfaceName(iface);
    m_connectSetting->setIpConfigType(IPADDRESS_V4, CONFIG_IP_DHCP);

    m_connectSetting->dumpInfo();
    QVERIFY2(true, "dumpInfo executed without crash");
}

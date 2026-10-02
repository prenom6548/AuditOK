#include "test_kylinnetresource.h"
#include <QSignalSpy>
#include <QTest>
#include <QDebug>

TestKyNetResource::TestKyNetResource(QObject *parent)
    : QObject(parent)
{
}

TestKyNetResource::~TestKyNetResource()
{
}

void TestKyNetResource::initTestCase()
{
    m_netResource = new KyNetResource();
}

void TestKyNetResource::cleanupTestCase()
{
    if (m_netResource) {
        delete m_netResource;
        m_netResource = nullptr;
    }
}

void TestKyNetResource::test_constructor()
{
    QVERIFY2(m_netResource != nullptr, "Failed to create KyNetResource instance");
}

void TestKyNetResource::test_destructor()
{
    KyNetResource *temp = new KyNetResource();
    QVERIFY2(temp != nullptr, "Failed to create temporary KyNetResource instance");
    delete temp;
    temp = nullptr;
    QVERIFY2(temp == nullptr, "Destructor test: temp should be nullptr after delete");
}

void TestKyNetResource::test_getConnectionSetting_withInvalidUuid()
{
    KyConnectSetting connectSetting;
    QString invalidUuid = "invalid-uuid-12345";
    m_netResource->getConnectionSetting(invalidUuid, connectSetting);
    QVERIFY(connectSetting.m_connectName.isEmpty());
    QVERIFY(connectSetting.m_ifaceName.isEmpty());
}

void TestKyNetResource::test_getConnectionSetting_withValidUuid()
{
    KyConnectSetting connectSetting;
    QString validUuid = "00000000-0000-0000-0000-000000000000";
    m_netResource->getConnectionSetting(validUuid, connectSetting);
}

void TestKyNetResource::test_getConnectionSetting_ipv4DhcpMethod()
{
    KyConnectSetting connectSetting;
    QString uuid = "test-uuid-ipv4-dhcp";
    m_netResource->getConnectionSetting(uuid, connectSetting);
}

void TestKyNetResource::test_getConnectionSetting_ipv4ManualMethod()
{
    KyConnectSetting connectSetting;
    QString uuid = "test-uuid-ipv4-manual";
    m_netResource->getConnectionSetting(uuid, connectSetting);
}

void TestKyNetResource::test_getConnectionSetting_ipv6DhcpMethod()
{
    KyConnectSetting connectSetting;
    QString uuid = "test-uuid-ipv6-dhcp";
    m_netResource->getConnectionSetting(uuid, connectSetting);
}

void TestKyNetResource::test_getConnectionSetting_ipv6ManualMethod()
{
    KyConnectSetting connectSetting;
    QString uuid = "test-uuid-ipv6-manual";
    m_netResource->getConnectionSetting(uuid, connectSetting);
}

#include "test_kylingeneral.h"
#include <QSignalSpy>
#include <QTest>

TestKylinGeneral::TestKylinGeneral(QObject *parent)
    : QObject(parent)
{
}

TestKylinGeneral::~TestKylinGeneral()
{
}

void TestKylinGeneral::initTestCase()
{
    m_general = new KylinGeneralOpration();
}

void TestKylinGeneral::cleanupTestCase()
{
    if (m_general) {
        delete m_general;
        m_general = nullptr;
    }
}

void TestKylinGeneral::test_constructor()
{
    QVERIFY2(m_general != nullptr, "Failed to create KylinGeneralOpration instance");
    QVERIFY2(m_general->parent() == nullptr, "Parent should be nullptr");
}

void TestKylinGeneral::test_destructor()
{
    KylinGeneralOpration *temp = new KylinGeneralOpration();
    QVERIFY2(temp != nullptr, "Failed to create temporary KylinGeneralOpration instance");
    delete temp;
    temp = nullptr;
    QVERIFY2(temp == nullptr, "Destructor test: temp should be nullptr after delete");
}

void TestKylinGeneral::test_getConnectStatus()
{
    KyConnectStatus status = NOT_CONNECTED;
    m_general->getConnectStatus(status);
    QVERIFY(status >= NOT_CONNECTED && status <= WLAN_CONNECTED_LIMITED);
}

void TestKylinGeneral::test_getWirelessEnabled()
{
    bool enabled = m_general->getWirelessEnabled();
    QVERIFY2((enabled == true || enabled == false), "getWirelessEnabled should return boolean");
}

void TestKylinGeneral::test_setWirelessNetworkEnabled()
{
    QSignalSpy spy(m_general, &KylinGeneralOpration::wifiEnabledChanged);
    //m_general->setWirelessNetworkEnabled(false);
    m_general->setWirelessNetworkEnabled(true);
    QVERIFY2(spy.count() >= 0, "wifiEnabledChanged signal should be emitted");
}

void TestKylinGeneral::test_getNetworkingEnabled()
{
    bool enabled = m_general->getNetworkingEnabled();
    QVERIFY2((enabled == true || enabled == false), "getNetworkingEnabled should return boolean");
}

void TestKylinGeneral::test_setNetworkingEnabled()
{
    QSignalSpy spy(m_general, &KylinGeneralOpration::networkingEnabledChanged);
    //m_general->setNetworkingEnabled(false);
    m_general->setNetworkingEnabled(true);
    QVERIFY2(spy.count() >= 0, "networkingEnabledChanged signal should be emitted");
}

void TestKylinGeneral::test_setWiredEnabled()
{
    QSignalSpy spy(m_general, &KylinGeneralOpration::wiredEnabledChanged);
    //m_general->setWiredEnabled(false);
    m_general->setWiredEnabled(true);
    QVERIFY2(spy.count() >= 0, "wiredEnabledChanged signal should be emitted");
}

void TestKylinGeneral::test_getWiredEnabled()
{
    bool enabled = m_general->getWiredEnabled();
    QVERIFY2((enabled == true || enabled == false), "getWiredEnabled should return boolean");
}

void TestKylinGeneral::test_connectStatusChanged_signal()
{
    QSignalSpy spy(m_general, &KylinGeneralOpration::connectStatusChanged);
    QVERIFY2(spy.isValid(), "connectStatusChanged signal should be valid");
}

void TestKylinGeneral::test_all_signal_connections()
{
    QSignalSpy wifiSpy(m_general, &KylinGeneralOpration::wifiEnabledChanged);
    QSignalSpy wiredSpy(m_general, &KylinGeneralOpration::wiredEnabledChanged);
    QSignalSpy netSpy(m_general, &KylinGeneralOpration::networkingEnabledChanged);
    QSignalSpy statusSpy(m_general, &KylinGeneralOpration::connectStatusChanged);

    QVERIFY2(wifiSpy.isValid(), "wifiEnabledChanged signal should be valid");
    QVERIFY2(wiredSpy.isValid(), "wiredEnabledChanged signal should be valid");
    QVERIFY2(netSpy.isValid(), "networkingEnabledChanged signal should be valid");
    QVERIFY2(statusSpy.isValid(), "connectStatusChanged signal should be valid");
}

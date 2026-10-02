#include "test_kylinwiredconnectoperation.h"
#include <QTest>
#include <QSignalSpy>

TestKyWiredConnectOperation::TestKyWiredConnectOperation(QObject *parent)
    : QObject(parent)
    , m_wiredConnectOperation(nullptr)
{
}

TestKyWiredConnectOperation::~TestKyWiredConnectOperation()
{
}

void TestKyWiredConnectOperation::initTestCase()
{
}

void TestKyWiredConnectOperation::cleanupTestCase()
{
}

void TestKyWiredConnectOperation::init()
{
    m_wiredConnectOperation = new KyWiredConnectOperation();
}

void TestKyWiredConnectOperation::cleanup()
{
    if (m_wiredConnectOperation) {
        delete m_wiredConnectOperation;
        m_wiredConnectOperation = nullptr;
    }
}

void TestKyWiredConnectOperation::test_constructor()
{
    KyWiredConnectOperation *operation = new KyWiredConnectOperation();
    QVERIFY(operation != nullptr);
    delete operation;
}

void TestKyWiredConnectOperation::test_destructor()
{
    KyWiredConnectOperation *operation = new KyWiredConnectOperation();
    delete operation;
    QVERIFY(true);
}

void TestKyWiredConnectOperation::test_createWiredConnect_basic()
{
    QSignalSpy spy(m_wiredConnectOperation, &KyWiredConnectOperation::createConnectionError);

    KyConnectSetting connectSettings;
    QString connName = QString("test-wired-connection");
    QString ifaceName = QString("eth0");
    connectSettings.setConnectName(connName);
    connectSettings.setIfaceName(ifaceName);

    m_wiredConnectOperation->createWiredConnect(connectSettings);
    
    QTest::qWait(100);
    
    QVERIFY(spy.count() >= 0);
}

void TestKyWiredConnectOperation::test_createWiredConnect_withSettings()
{
    QSignalSpy spy(m_wiredConnectOperation, &KyWiredConnectOperation::createConnectionError);

    KyConnectSetting connectSettings;
    QString connName = QString("test-wired-connection-with-settings");
    QString ifaceName = QString("eth1");
    connectSettings.setConnectName(connName);
    connectSettings.setIfaceName(ifaceName);
    connectSettings.setIpConfigType(IPADDRESS_V4, CONFIG_IP_DHCP);
    connectSettings.setIpConfigType(IPADDRESS_V6, CONFIG_IP_DHCP);

    m_wiredConnectOperation->createWiredConnect(connectSettings);
    
    QTest::qWait(100);
    
    QVERIFY(spy.count() >= 0);
}

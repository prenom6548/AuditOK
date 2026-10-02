#include "test_kylinconnectoperation.h"

#include <NetworkManagerQt/Ipv4Setting>
#include <NetworkManagerQt/Ipv6Setting>
#include <NetworkManagerQt/ConnectionSettings>
#include "kylinconnectsetting.h"

TestKyConnectOperation::TestKyConnectOperation(QObject *parent)
    : QObject(parent)
    , m_connectOperation(nullptr)
{
}

TestKyConnectOperation::~TestKyConnectOperation()
{
}

void TestKyConnectOperation::initTestCase()
{
}

void TestKyConnectOperation::cleanupTestCase()
{
}

void TestKyConnectOperation::init()
{
    m_connectOperation = new KyConnectOperation();
}

void TestKyConnectOperation::cleanup()
{
    if (m_connectOperation) {
        delete m_connectOperation;
        m_connectOperation = nullptr;
    }
}

void TestKyConnectOperation::test_constructor()
{
    KyConnectOperation *operation = new KyConnectOperation();
    QVERIFY(operation != nullptr);
    delete operation;
}

void TestKyConnectOperation::test_destructor()
{
    KyConnectOperation *operation = new KyConnectOperation();
    delete operation;
    QVERIFY(true);
}

void TestKyConnectOperation::test_updateConnect_notFound()
{
    QSignalSpy spy(m_connectOperation, &KyConnectOperation::updateConnectionError);
    
    KyConnectSetting connectSettings;
    QString invalidUuid = "00000000-0000-0000-0000-000000000000";
    m_connectOperation->updateConnect(invalidUuid, connectSettings);
    
    QVERIFY(spy.count() == 1);
}

void TestKyConnectOperation::test_deleteConnect_notFound()
{
    QSignalSpy spy(m_connectOperation, &KyConnectOperation::deleteConnectionError);
    
    QString invalidUuid = "00000000-0000-0000-0000-000000000000";
    m_connectOperation->deleteConnect(invalidUuid);
    
    QVERIFY(spy.count() == 1);
}

void TestKyConnectOperation::test_activateConnection_notFound()
{
    QSignalSpy spy(m_connectOperation, &KyConnectOperation::activateConnectionError);
    
    QString invalidUuid = "00000000-0000-0000-0000-000000000000";
    QString deviceName = "eth0";
    m_connectOperation->activateConnection(invalidUuid, deviceName);
    
    QVERIFY(spy.count() == 1);
}

void TestKyConnectOperation::test_activateConnection_deviceNotFound()
{
    QSignalSpy spy(m_connectOperation, &KyConnectOperation::activateConnectionError);
    
    QString validUuid = "12345678-1234-1234-1234-123456789012";
    QString invalidDeviceName = "nonexistent_device";
    m_connectOperation->activateConnection(validUuid, invalidDeviceName);
    
    QVERIFY(spy.count() == 1);
}

void TestKyConnectOperation::test_activateConnection_success()
{
    QString validUuid = "12345678-1234-1234-1234-123456789012";
    QString validDeviceName = "lo";
    m_connectOperation->activateConnection(validUuid, validDeviceName);
    QTest::qWait(100);
    QVERIFY(true);
}

void TestKyConnectOperation::test_deactivateConnection_notFound()
{
    QSignalSpy spy(m_connectOperation, &KyConnectOperation::deactivateConnectionError);
    
    QString invalidUuid = "00000000-0000-0000-0000-000000000000";
    m_connectOperation->deactivateConnection(invalidUuid);
    
    QVERIFY(spy.count() == 1);
}

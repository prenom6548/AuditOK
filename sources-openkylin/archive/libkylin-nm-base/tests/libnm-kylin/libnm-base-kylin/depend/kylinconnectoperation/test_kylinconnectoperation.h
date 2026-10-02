#ifndef TEST_KYLINCONNECTOPERATION_H
#define TEST_KYLINCONNECTOPERATION_H

#include <QObject>
#include <QTest>
#include <QSignalSpy>
#include <QDebug>

#include "kylinconnectoperation.h"

class TestKyConnectOperation : public QObject
{
    Q_OBJECT
public:
    explicit TestKyConnectOperation(QObject *parent = nullptr);
    ~TestKyConnectOperation();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void test_constructor();
    void test_destructor();
    void test_updateConnect_notFound();
    void test_deleteConnect_notFound();
    void test_activateConnection_notFound();
    void test_activateConnection_deviceNotFound();
    void test_activateConnection_success();
    void test_deactivateConnection_notFound();

private:
    KyConnectOperation *m_connectOperation;
};

#endif // TEST_KYLINCONNECTOPERATION_H

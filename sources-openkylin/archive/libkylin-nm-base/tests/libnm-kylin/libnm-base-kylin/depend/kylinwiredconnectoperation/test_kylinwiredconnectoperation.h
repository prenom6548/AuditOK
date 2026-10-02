#ifndef TEST_KYLINWIREDCONNECTOPERATION_H
#define TEST_KYLINWIREDCONNECTOPERATION_H

#include <QObject>
#include <QString>
#include "kylinwiredconnectoperation.h"
#include "kylinconnectsetting.h"

class TestKyWiredConnectOperation : public QObject
{
    Q_OBJECT
public:
    explicit TestKyWiredConnectOperation(QObject *parent = nullptr);
    ~TestKyWiredConnectOperation();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void test_constructor();
    void test_destructor();
    void test_createWiredConnect_basic();
    void test_createWiredConnect_withSettings();

private:
    KyWiredConnectOperation *m_wiredConnectOperation;
};

#endif // TEST_KYLINWIREDCONNECTOPERATION_H
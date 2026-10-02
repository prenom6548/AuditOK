#ifndef TEST_KYLINGENERAL_H
#define TEST_KYLINGENERAL_H

#include <QObject>
#include "kylingeneral.h"

class TestKylinGeneral : public QObject
{
    Q_OBJECT

public:
    explicit TestKylinGeneral(QObject *parent = nullptr);
    ~TestKylinGeneral();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void test_constructor();
    void test_destructor();
    void test_getConnectStatus();
    void test_getWirelessEnabled();
    void test_setWirelessNetworkEnabled();
    void test_getNetworkingEnabled();
    void test_setNetworkingEnabled();
    void test_setWiredEnabled();
    void test_getWiredEnabled();
    void test_connectStatusChanged_signal();
    void test_all_signal_connections();

private:
    KylinGeneralOpration *m_general = nullptr;
};

#endif // TEST_KYLINGENERAL_H

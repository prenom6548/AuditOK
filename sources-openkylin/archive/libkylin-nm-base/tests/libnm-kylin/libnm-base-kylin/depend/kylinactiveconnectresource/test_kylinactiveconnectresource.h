#ifndef TEST_KYLINACTIVECONNECTRESOURCE_H
#define TEST_KYLINACTIVECONNECTRESOURCE_H

#include <QObject>
#include <QSignalSpy>
#include "kylinactiveconnectresource.h"

class TestKylinActiveConnectResource : public QObject
{
    Q_OBJECT

public:
    explicit TestKylinActiveConnectResource(QObject *parent = nullptr);
    ~TestKylinActiveConnectResource();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void test_constructor();
    void test_destructor();
    void test_getActiveConnectionItem_nullptr();
    void test_getActiveConnectionItem_notActivated();
    void test_getActiveConnectionItem_normal();
    void test_getActiveConnectionList_empty();
    void test_getActiveConnectionList_normal();
    void test_getActiveConnectIpInfo_invalidUuid();
    void test_getActiveConnectIpInfo_validUuid();
    void test_getActiveConnectDnsInfo_invalidUuid();
    void test_getActiveConnectDnsInfo_validUuid();
    void test_wiredConnectIsActived_emptyList();
    void test_wiredConnectIsActived_notWired();
    void test_wiredConnectIsActived_notActivated();
    void test_wiredConnectIsActived_activated();
    void test_wirelessConnectIsActived_emptyList();
    void test_wirelessConnectIsActived_notWireless();
    void test_wirelessConnectIsActived_activated();
    void test_getAcivateWifiSignal();

    void test_wiredStateChange_signal();
    void test_wirelessStateChange_signal();

private:
    KyActiveConnectResourse *m_activeConnect = nullptr;
};

#endif // TEST_KYLINACTIVECONNECTRESOURCE_H

#ifndef TEST_KYLINNETWORKRESOURCEMANAGER_H
#define TEST_KYLINNETWORKRESOURCEMANAGER_H

#include <QObject>
#include <QTest>
#include <QSignalSpy>
#include <QDebug>

#include "kylinnetworkresourcemanager.h"

class TestKyNetworkResourceManager : public QObject
{
    Q_OBJECT
public:
    explicit TestKyNetworkResourceManager(QObject *parent = nullptr);
    ~TestKyNetworkResourceManager();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void test_getInstance();
    void test_Release();
    void test_constructor();
    void test_destructor();
    void test_findActiveConnectionByPath_emptyPath();
    void test_findActiveConnectionByPath_notFound();
    void test_findActiveConnectByUuid_emptyUuid();
    void test_findActiveConnectByUuid_notFound();
    void test_findConnectByUuid_emptyUuid();
    void test_findConnectByUuid_notFound();
    void test_findDeviceByUni_emptyUni();
    void test_findDeviceByUni_notFound();
    void test_findDeviceByName_emptyName();
    void test_findDeviceByName_notFound();
    void test_findWifiNetwork_emptyParams();
    void test_findWifiNetwork_notFound();
    void test_getActiveConnectList();
    void test_getConnectList();
    void test_getNetworkDeviceList();
    void test_getWifiNetworkList();
    void test_isActiveConnection_emptyUuid();
    void test_isActiveConnection_notFound();
    void test_isActivatingConnection_emptyUuid();
    void test_isActivatingConnection_notFound();
    void test_getConnectivity();
    void test_NetworkManagerIsInited();
    void test_isWiredConnect_emptyPath();
    void test_isWiredConnect_notWired();
    void test_isApConnection_nullptr();
    void test_isApConnection_notWireless();
    void test_isApConnection_notApMode();

private:
    KyNetworkResourceManager *m_manager;
};

#endif // TEST_KYLINNETWORKRESOURCEMANAGER_H

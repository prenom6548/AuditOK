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

private Q_SLOTS:

    void initTestCase();
    void cleanupTestCase();

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
    void test_findWifiNetwork_emptySsid();
    void test_findWifiNetwork_emptyUni();
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
    void test_requestScan_nullptr();
    void test_onServiceAppear();
    void test_onServiceAppear_differentInterface();
    void test_onServiceAppear_existingOwner();
    void test_onPropertiesChanged();
    void test_onPropertiesChanged_empty();
    void test_onPropertiesChanged_wiredDisabled();
    void test_onConnectivityChanged();
    void test_onConnectivityChanged_none();
    void test_onConnectivityChanged_limited();
    void test_onPrimaryConnectionTypeChanged();
    void test_onPrimaryConnectionTypeChanged_wired();
    void test_signal_wiredConnectionAdd();
    void test_signal_wiredConnectionUpdate();
    void test_signal_connectionAdd();
    void test_connectionUpdate();
    void test_connectionRemove();
    void test_signal_wiredDeviceAdd();
    void test_signal_wiredDeviceUpdate();
    void test_signal_wirelessDeviceAdd();
    void test_signal_wirelessDeviceUpdate();
    void test_signal_deviceAdd();
    void test_deviceUpdate();
    void test_signal_deviceRemove();
    void test_deviceManagedChange();
    void test_signal_deviceActiveChange();
    void test_deviceCarrierChange();
    void test_deviceBitRateChange();
    void test_deviceMacAddressChange();
    void test_deviceStateChange();
    void test_signal_wifiNetworkRemoved();
    void test_wifiNetworkAdded();
    void test_signal_wifiNetworkPropertyChange();
    void test_signal_wifiNetworkDeviceDisappear();
    void test_signal_wirelessConnectionAdd();
    void test_signal_wirelessConnectionUpdate();
    void test_signal_activeConnectionAdd();
    void test_signal_activeConnectionRemove();
    void test_signal_wiredConnectStateChange();
    void test_signal_wirelessConnectStateChange();
    void test_signal_wirelessApConnectStateChange();
    void test_signal_connectivityChanged();
    void test_signal_wifiEnabledChanged();
    void test_signal_wiredEnabledChanged();
    void test_signal_networkingEnabledChanged();
    void test_signal_primaryConnectionTypeChanged();
    void test_wirelessActiveStateChange();
    void test_wiredActiveStateChange();
    void test_connectActiveStateChange();
    void test_wifiNetworkAdd_Device();
    void test_wifiNetworkUpdate_nullNet();
    void test_wifiNetworkRemove_nullDevice();
    void test_onDeviceAdded();
    void test_updateDeviceName();
    void test_onDeviceRemoved_emptyUni();
    void test_onActiveConnectionAdded_invalidPath();
    void test_onActiveConnectionRemoved_emptyPath();
    void test_onConnectionAdded_invalidPath();
    void test_onConnectionRemoved_emptyPath();
    void test_onConnectionRemoved_validPath();
    void test_connectionDump();
    void test_Release();

private:
    KyNetworkResourceManager *m_manager;
};

#endif // TEST_KYLINNETWORKRESOURCEMANAGER_H

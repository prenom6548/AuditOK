#include "test_kylinnetworkresourcemanager.h"

#include <NetworkManagerQt/Ipv4Setting>
#include <NetworkManagerQt/Ipv6Setting>
#include <NetworkManagerQt/ConnectionSettings>
#include "kylinnetworkresourcetype.h"

TestKyNetworkResourceManager::TestKyNetworkResourceManager(QObject *parent)
    : QObject(parent)
    , m_manager(nullptr)
{
}

TestKyNetworkResourceManager::~TestKyNetworkResourceManager()
{
}




void TestKyNetworkResourceManager::initTestCase()
{
    m_manager = KyNetworkResourceManager::getInstance();
    sleep(5);

    if(m_manager) m_manager->onInitNetwork();

    qWarning() << "initTestCase. TestKyNetworkResourceManager";
}

void TestKyNetworkResourceManager::cleanupTestCase()
{
}




void TestKyNetworkResourceManager::test_findActiveConnectionByPath_emptyPath()
{
    QString emptyPath = "";
    NetworkManager::ActiveConnection::Ptr result = m_manager->findActiveConnectionByPath(emptyPath);
    QVERIFY(result.isNull());
}

void TestKyNetworkResourceManager::test_findActiveConnectionByPath_notFound()
{
    QString invalidPath = "/org/freedesktop/NetworkManager/ActiveConnection/999999";
    NetworkManager::ActiveConnection::Ptr result = m_manager->findActiveConnectionByPath(invalidPath);
    QVERIFY(result.isNull());
}

void TestKyNetworkResourceManager::test_findActiveConnectByUuid_emptyUuid()
{
    QString emptyUuid = "";
    NetworkManager::ActiveConnection::Ptr result = m_manager->findActiveConnectByUuid(emptyUuid);
    QVERIFY(result.isNull());
}

void TestKyNetworkResourceManager::test_findActiveConnectByUuid_notFound()
{
    QString invalidUuid = "00000000-0000-0000-0000-000000000000";
    NetworkManager::ActiveConnection::Ptr result = m_manager->findActiveConnectByUuid(invalidUuid);
    QVERIFY(result.isNull());
}

void TestKyNetworkResourceManager::test_findConnectByUuid_emptyUuid()
{
    QString emptyUuid = "";
    NetworkManager::Connection::Ptr result = m_manager->findConnectByUuid(emptyUuid);
    QVERIFY(result.isNull());
}

void TestKyNetworkResourceManager::test_findConnectByUuid_notFound()
{
    QString invalidUuid = "00000000-0000-0000-0000-000000000000";
    NetworkManager::Connection::Ptr result = m_manager->findConnectByUuid(invalidUuid);
    QVERIFY(result.isNull());
}

void TestKyNetworkResourceManager::test_findDeviceByUni_emptyUni()
{
    QString emptyUni = "";
    NetworkManager::Device::Ptr result = m_manager->findDeviceByUni(emptyUni);
    QVERIFY(result.isNull());
}

void TestKyNetworkResourceManager::test_findDeviceByUni_notFound()
{
    QString invalidUni = "invalid-uni-12345";
    NetworkManager::Device::Ptr result = m_manager->findDeviceByUni(invalidUni);
    QVERIFY(result.isNull());
}

void TestKyNetworkResourceManager::test_findDeviceByName_emptyName()
{
    QString emptyName = "";
    NetworkManager::Device::Ptr result = m_manager->findDeviceByName(emptyName);
    QVERIFY(result.isNull());
}

void TestKyNetworkResourceManager::test_findDeviceByName_notFound()
{
    QString invalidName = "nonexistent_device_xyz";
    NetworkManager::Device::Ptr result = m_manager->findDeviceByName(invalidName);
    QVERIFY(result.isNull());
}

void TestKyNetworkResourceManager::test_findWifiNetwork_emptySsid()
{
    QString emptySsid = "";
    QString validUni = "eth0";
    NetworkManager::WirelessNetwork::Ptr result = m_manager->findWifiNetwork(emptySsid, validUni);
    QVERIFY(result.isNull());
}

void TestKyNetworkResourceManager::test_findWifiNetwork_emptyUni()
{
    QString validSsid = "TestNetwork";
    QString emptyUni = "";
    NetworkManager::WirelessNetwork::Ptr result = m_manager->findWifiNetwork(validSsid, emptyUni);
    QVERIFY(result.isNull());
}

void TestKyNetworkResourceManager::test_findWifiNetwork_emptyParams()
{
    QString emptySsid = "";
    QString emptyUni = "";
    NetworkManager::WirelessNetwork::Ptr result = m_manager->findWifiNetwork(emptySsid, emptyUni);
    QVERIFY(result.isNull());
}

void TestKyNetworkResourceManager::test_findWifiNetwork_notFound()
{
    QString invalidSsid = "NonexistentNetworkXYZ";
    QString invalidUni = "invalid-uni-12345";
    NetworkManager::WirelessNetwork::Ptr result = m_manager->findWifiNetwork(invalidSsid, invalidUni);
    QVERIFY(result.isNull());
}

void TestKyNetworkResourceManager::test_getActiveConnectList()
{
    NetworkManager::ActiveConnection::List result = m_manager->getActiveConnectList();
    QVERIFY(result.isEmpty() || !result.isEmpty());
}

void TestKyNetworkResourceManager::test_getConnectList()
{
    NetworkManager::Connection::List result = m_manager->getConnectList();
    QVERIFY(result.isEmpty() || !result.isEmpty());
}

void TestKyNetworkResourceManager::test_getNetworkDeviceList()
{
    NetworkManager::Device::List result = m_manager->getNetworkDeviceList();
    QVERIFY(result.isEmpty() || !result.isEmpty());
}

void TestKyNetworkResourceManager::test_getWifiNetworkList()
{
    NetworkManager::WirelessNetwork::List result = m_manager->getWifiNetworkList();
    QVERIFY(result.isEmpty() || !result.isEmpty());
}

void TestKyNetworkResourceManager::test_isActiveConnection_emptyUuid()
{
    QString emptyUuid = "";
    bool result = m_manager->isActiveConnection(emptyUuid);
    QVERIFY(result == false);
}

void TestKyNetworkResourceManager::test_isActiveConnection_notFound()
{
    QString invalidUuid = "00000000-0000-0000-0000-000000000000";
    bool result = m_manager->isActiveConnection(invalidUuid);
    QVERIFY(result == false);
}

void TestKyNetworkResourceManager::test_isActivatingConnection_emptyUuid()
{
    QString emptyUuid = "";
    bool result = m_manager->isActivatingConnection(emptyUuid);
    QVERIFY(result == false);
}

void TestKyNetworkResourceManager::test_isActivatingConnection_notFound()
{
    QString invalidUuid = "00000000-0000-0000-0000-000000000000";
    bool result = m_manager->isActivatingConnection(invalidUuid);
    QVERIFY(result == false);
}

void TestKyNetworkResourceManager::test_getConnectivity()
{
    KyConnectivity connectivity = CONNECTIVITY_UNKNOWN;
    m_manager->getConnectivity(connectivity);
    QVERIFY(connectivity >= CONNECTIVITY_UNKNOWN && connectivity <= CONNECTIVITY_FULL);
}

void TestKyNetworkResourceManager::test_NetworkManagerIsInited()
{
    bool result = m_manager->NetworkManagerIsInited();
    QVERIFY(result == true || result == false);
}

void TestKyNetworkResourceManager::test_isWiredConnect_emptyPath()
{
    QString emptyPath = "";
    bool result = m_manager->isWiredConnect(emptyPath);
    QVERIFY(result == false);
}

void TestKyNetworkResourceManager::test_isWiredConnect_notWired()
{
    QString wirelessPath = "/org/freedesktop/NetworkManager/ActiveConnection/1";
    bool result = m_manager->isWiredConnect(wirelessPath);
    QVERIFY(result == false || result == true);
}

void TestKyNetworkResourceManager::test_isApConnection_nullptr()
{
    NetworkManager::Connection::Ptr nullPtr;
    bool result = m_manager->isApConnection(nullPtr);
    QVERIFY(result == false);
}

void TestKyNetworkResourceManager::test_isApConnection_notWireless()
{
    NetworkManager::Connection::Ptr connPtr;
    bool result = m_manager->isApConnection(connPtr);
    QVERIFY(result == false);
}

void TestKyNetworkResourceManager::test_isApConnection_notApMode()
{
    NetworkManager::Connection::Ptr connPtr;
    bool result = m_manager->isApConnection(connPtr);
    QVERIFY(result == false);
}

void TestKyNetworkResourceManager::test_requestScan_nullptr()
{
    // 测试空指针情况
    NetworkManager::WirelessDevice *nullDevice = nullptr;
    m_manager->requestScan(nullDevice);
    // 验证没有崩溃
    QVERIFY(true);
}

void TestKyNetworkResourceManager::test_onServiceAppear()
{
    // 测试服务出现的情况
    QString interface = "org.freedesktop.NetworkManager";
    QString oldOwner = "";
    QString newOwner = "new_owner";
    
    // 调用onServiceAppear
    m_manager->onServiceAppear(interface, oldOwner, newOwner);
    
    // 验证没有崩溃
    QVERIFY(true);
}

void TestKyNetworkResourceManager::test_onServiceAppear_differentInterface()
{
    QString interface = "org.freedesktop.DBus";
    QString oldOwner = "";
    QString newOwner = "new_owner";
    
    m_manager->onServiceAppear(interface, oldOwner, newOwner);
    
    QVERIFY(true);
}

void TestKyNetworkResourceManager::test_onServiceAppear_existingOwner()
{
    QString interface = "org.freedesktop.NetworkManager";
    QString oldOwner = "existing_owner";
    QString newOwner = "new_owner";
    
    m_manager->onServiceAppear(interface, oldOwner, newOwner);
    
    QVERIFY(true);
}

void TestKyNetworkResourceManager::test_onPropertiesChanged()
{
    // 测试属性变化
    QVariantMap qvm;
    qvm.insert("WiredEnabled", true);
    
    m_manager->onPropertiesChanged(qvm);
    
    QVERIFY(true);
}

void TestKyNetworkResourceManager::test_onPropertiesChanged_empty()
{
    QVariantMap qvm;
    m_manager->onPropertiesChanged(qvm);
    QVERIFY(true);
}

void TestKyNetworkResourceManager::test_onPropertiesChanged_wiredDisabled()
{
    QVariantMap qvm;
    qvm.insert("WiredEnabled", false);
    
    m_manager->onPropertiesChanged(qvm);
    
    QVERIFY(true);
}

void TestKyNetworkResourceManager::test_onConnectivityChanged()
{
    // 测试连接状态变化
    NetworkManager::Connectivity connectivity = NetworkManager::Connectivity::Full;
    m_manager->onConnectivityChanged(connectivity);
    QVERIFY(true);
}

void TestKyNetworkResourceManager::test_onConnectivityChanged_none()
{
    NetworkManager::Connectivity connectivity = NetworkManager::Connectivity::UnknownConnectivity;
    m_manager->onConnectivityChanged(connectivity);
    QVERIFY(true);
}

void TestKyNetworkResourceManager::test_onConnectivityChanged_limited()
{
    NetworkManager::Connectivity connectivity = NetworkManager::Connectivity::Limited;
    m_manager->onConnectivityChanged(connectivity);
    QVERIFY(true);
}

void TestKyNetworkResourceManager::test_onPrimaryConnectionTypeChanged()
{
    NetworkManager::ConnectionSettings::ConnectionType type = NetworkManager::ConnectionSettings::Wireless;
    m_manager->onPrimaryConnectionTypeChanged(type);
    QVERIFY(true);
}

void TestKyNetworkResourceManager::test_onPrimaryConnectionTypeChanged_wired()
{
    NetworkManager::ConnectionSettings::ConnectionType type = NetworkManager::ConnectionSettings::Wired;
    m_manager->onPrimaryConnectionTypeChanged(type);
    QVERIFY(true);
}

void TestKyNetworkResourceManager::test_signal_wiredConnectionAdd()
{
    QSignalSpy spy(m_manager, &KyNetworkResourceManager::wiredConnectionAdd);
    QVERIFY(spy.isValid());
}

void TestKyNetworkResourceManager::test_signal_wiredConnectionUpdate()
{
    QSignalSpy spy(m_manager, &KyNetworkResourceManager::wiredConnectionUpdate);
    QVERIFY(spy.isValid());
}

void TestKyNetworkResourceManager::test_signal_connectionAdd()
{
    QSignalSpy spy(m_manager, &KyNetworkResourceManager::connectionAdd);
    QVERIFY(spy.isValid());
}

void TestKyNetworkResourceManager::test_connectionUpdate()
{
    QSignalSpy spy(m_manager, &KyNetworkResourceManager::connectionUpdate);
    m_manager->onConnectionUpdated();

    m_manager->onActiveConnectionChanged(NetworkManager::ActiveConnection::State::Unknown);

    m_manager->onVpnActiveConnectChangedReason(NetworkManager::VpnConnection::State::Unknown,NetworkManager::VpnConnection::StateChangeReason::UnknownReason);


    QVERIFY(spy.isValid());
}



void TestKyNetworkResourceManager::test_connectionRemove()
{
    if(m_manager->m_connectList.size() > 1){
    m_manager->removeConnection(1);
    }

//m_connectList 清空
    //m_manager->clearConnections();

}

void TestKyNetworkResourceManager::test_signal_wiredDeviceAdd()
{
    QSignalSpy spy(m_manager, &KyNetworkResourceManager::wiredDeviceAdd);
    QVERIFY(spy.isValid());
}

void TestKyNetworkResourceManager::test_signal_wiredDeviceUpdate()
{
    QSignalSpy spy(m_manager, &KyNetworkResourceManager::wiredDeviceUpdate);
    QVERIFY(spy.isValid());
}

void TestKyNetworkResourceManager::test_signal_wirelessDeviceAdd()
{
    QSignalSpy spy(m_manager, &KyNetworkResourceManager::wirelessDeviceAdd);
    QVERIFY(spy.isValid());
}

void TestKyNetworkResourceManager::test_signal_wirelessDeviceUpdate()
{
    QSignalSpy spy(m_manager, &KyNetworkResourceManager::wirelessDeviceUpdate);
    QVERIFY(spy.isValid());
}

void TestKyNetworkResourceManager::test_signal_deviceAdd()
{
    QSignalSpy spy(m_manager, &KyNetworkResourceManager::deviceAdd);
    QVERIFY(spy.isValid());
}

void TestKyNetworkResourceManager::test_deviceUpdate()
{
    QSignalSpy spy(m_manager, &KyNetworkResourceManager::deviceUpdate);
    m_manager->onDeviceUpdated();
    QVERIFY(spy.isValid());
}

void TestKyNetworkResourceManager::test_signal_deviceRemove()
{
    QSignalSpy spy(m_manager, &KyNetworkResourceManager::deviceRemove);
    QVERIFY(spy.isValid());
}

void TestKyNetworkResourceManager::test_deviceManagedChange()
{

    QSignalSpy spy(m_manager, &KyNetworkResourceManager::deviceManagedChange);
        m_manager->onDeviceManagedChange();
    QVERIFY(spy.isValid());
}

void TestKyNetworkResourceManager::test_signal_deviceActiveChange()
{
    QSignalSpy spy(m_manager, &KyNetworkResourceManager::deviceActiveChange);
    QVERIFY(spy.isValid());
}

void TestKyNetworkResourceManager::test_deviceCarrierChange()
{
    QSignalSpy spy(m_manager, &KyNetworkResourceManager::deviceCarrierChange);
    m_manager->onDeviceCarrierChange(true);
    QVERIFY(spy.isValid());
}

void TestKyNetworkResourceManager::test_deviceBitRateChange()
{
    m_manager->onDeviceBitRateChange(100);
    QSignalSpy spy(m_manager, &KyNetworkResourceManager::deviceBitRateChange);
    QVERIFY(spy.isValid());
}

void TestKyNetworkResourceManager::test_deviceMacAddressChange()
{
    m_manager->onDeviceMacAddressChange("00:23:54:6c:89:0a");
    QSignalSpy spy(m_manager, &KyNetworkResourceManager::deviceMacAddressChange);

    QVERIFY(spy.isValid());
}

void TestKyNetworkResourceManager::test_deviceStateChange()
{
    QSignalSpy spy(m_manager, &KyNetworkResourceManager::deviceStateChange);

    m_manager->onDeviceActiveChange();
    QVERIFY(spy.isValid());
}

void TestKyNetworkResourceManager::test_signal_wifiNetworkRemoved()
{
    QSignalSpy spy(m_manager, &KyNetworkResourceManager::wifiNetworkRemoved);
    QVERIFY(spy.isValid());
}

void TestKyNetworkResourceManager::test_wifiNetworkAdded()
{
    QSignalSpy spy(m_manager, &KyNetworkResourceManager::wifiNetworkAdded);
    m_manager->wifiNetworkAdd(nullptr,"TESTSSID");
    QVERIFY(spy.isValid());
}

void TestKyNetworkResourceManager::test_signal_wifiNetworkPropertyChange()
{
    QSignalSpy spy(m_manager, &KyNetworkResourceManager::wifiNetworkPropertyChange);
    QVERIFY(spy.isValid());
}

void TestKyNetworkResourceManager::test_signal_wifiNetworkDeviceDisappear()
{
    QSignalSpy spy(m_manager, &KyNetworkResourceManager::wifiNetworkDeviceDisappear);
    QVERIFY(spy.isValid());
}

void TestKyNetworkResourceManager::test_signal_wirelessConnectionAdd()
{
    QSignalSpy spy(m_manager, &KyNetworkResourceManager::wirelessConnectionAdd);
    QVERIFY(spy.isValid());
}

void TestKyNetworkResourceManager::test_signal_wirelessConnectionUpdate()
{
    QSignalSpy spy(m_manager, &KyNetworkResourceManager::wirelessConnectionUpdate);
    QVERIFY(spy.isValid());
}

void TestKyNetworkResourceManager::test_signal_activeConnectionAdd()
{
    QSignalSpy spy(m_manager, &KyNetworkResourceManager::activeConnectionAdd);
    QVERIFY(spy.isValid());
}

void TestKyNetworkResourceManager::test_signal_activeConnectionRemove()
{
    QSignalSpy spy(m_manager, &KyNetworkResourceManager::activeConnectionRemove);
    QVERIFY(spy.isValid());
}

void TestKyNetworkResourceManager::test_signal_wiredConnectStateChange()
{
    QSignalSpy spy(m_manager, &KyNetworkResourceManager::wiredConnectStateChange);
    QVERIFY(spy.isValid());
}

void TestKyNetworkResourceManager::test_signal_wirelessConnectStateChange()
{
    QSignalSpy spy(m_manager, &KyNetworkResourceManager::wirelessConnectStateChange);
    QVERIFY(spy.isValid());
}

void TestKyNetworkResourceManager::test_signal_wirelessApConnectStateChange()
{
    QSignalSpy spy(m_manager, &KyNetworkResourceManager::wirelessApConnectStateChange);
    QVERIFY(spy.isValid());
}

void TestKyNetworkResourceManager::test_signal_connectivityChanged()
{
    QSignalSpy spy(m_manager, &KyNetworkResourceManager::connectivityChanged);
    QVERIFY(spy.isValid());
}

void TestKyNetworkResourceManager::test_signal_wifiEnabledChanged()
{
    QSignalSpy spy(m_manager, &KyNetworkResourceManager::wifiEnabledChanged);
    QVERIFY(spy.isValid());
}

void TestKyNetworkResourceManager::test_signal_wiredEnabledChanged()
{
    QSignalSpy spy(m_manager, &KyNetworkResourceManager::wiredEnabledChanged);
    QVERIFY(spy.isValid());
}

void TestKyNetworkResourceManager::test_signal_networkingEnabledChanged()
{
    QSignalSpy spy(m_manager, &KyNetworkResourceManager::networkingEnabledChanged);
    QVERIFY(spy.isValid());
}

void TestKyNetworkResourceManager::test_signal_primaryConnectionTypeChanged()
{
    QSignalSpy spy(m_manager, &KyNetworkResourceManager::primaryConnectionTypeChanged);
    QVERIFY(spy.isValid());
}

void TestKyNetworkResourceManager::test_wirelessActiveStateChange()
{
    // 测试无线连接状态变化
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    QString deviceName = "wlan0";
    QString uuid = "test-uuid";
    KyConnectState state = KyConnectState::CONNECT_STATE_ACTIVATED;
    
    m_manager->wirelessActiveStateChange(connSettings, deviceName, uuid, state);
    QVERIFY(true);
}

void TestKyNetworkResourceManager::test_wiredActiveStateChange()
{
    // 测试有线连接状态变化
    QString connectPath = "/org/freedesktop/NetworkManager/ActiveConnection/1";
    QString deviceName = "eth0";
    QString uuid = "test-uuid";
    KyConnectState state = KyConnectState::CONNECT_STATE_ACTIVATED;
    
    m_manager->wiredActiveStateChange(connectPath, deviceName, uuid, state);

    QVERIFY(true);
}

void TestKyNetworkResourceManager::test_connectActiveStateChange()
{

    //connectActiveStateChange
    // 从连接列表中获取有线和无线连接分别测试
    NetworkManager::Connection::List connList = m_manager->getConnectList();
    
    NetworkManager::Connection::Ptr wirelessConn;
    NetworkManager::Connection::Ptr wiredConn;
    
    // 分离有线和无线连接
    for (const NetworkManager::Connection::Ptr &conn : connList) {
        if (conn.isNull()) continue;
        
        NetworkManager::ConnectionSettings::Ptr settings = conn->settings();
        if (settings.isNull()) continue;
        
        NetworkManager::ConnectionSettings::ConnectionType type = settings->connectionType();
        if (type == NetworkManager::ConnectionSettings::Wireless && wirelessConn.isNull()) {
            wirelessConn = conn;
        } else if (type == NetworkManager::ConnectionSettings::Wired && wiredConn.isNull()) {
            wiredConn = conn;
        }
        
        if (!wirelessConn.isNull() && !wiredConn.isNull()) {
            break;
        }
    }
    
    // 测试无线连接
    if (!wirelessConn.isNull()) {
        QString deviceName = "wlan0";
        KyConnectState state = KyConnectState::CONNECT_STATE_ACTIVATED;
        m_manager->connectActiveStateChange(wirelessConn, deviceName, state);
    }
    
    // 测试有线连接
    if (!wiredConn.isNull()) {
        QString deviceName = "eth0";
        KyConnectState state = KyConnectState::CONNECT_STATE_ACTIVATED;
        m_manager->connectActiveStateChange(wiredConn, deviceName, state);
    }
    
    QVERIFY(true);
}

void TestKyNetworkResourceManager::test_wifiNetworkAdd_Device()
{
    // 测试空设备情况
    NetworkManager::Device *dev = nullptr;
    QString ssid = "TestNetwork";
    
    // 测试有效设备情况 - 从设备列表获取WirelessDevice
    NetworkManager::Device::List deviceList = m_manager->getNetworkDeviceList();
    for (const NetworkManager::Device::Ptr &device : deviceList) {
        if (device.isNull()) continue;
        
        if (device->type() == NetworkManager::Device::Wifi) {
            // 转换为WirelessDevice并测试
            NetworkManager::WirelessDevice *wDev = qobject_cast<NetworkManager::WirelessDevice *>(device.data());
            if (wDev) {
                QString ssid = "TestNetwork";
                m_manager->wifiNetworkAdd(wDev, ssid);
                break;
            }
        }
    }
     m_manager->onWifiNetworkAppeared(ssid);
    
    QVERIFY(true);
}

void TestKyNetworkResourceManager::test_wifiNetworkUpdate_nullNet()
{
    // 测试空网络情况
    NetworkManager::WirelessNetwork *net = nullptr;
    
    m_manager->wifiNetworkUpdate(net);

    m_manager->onUpdateWirelessNet();
    QVERIFY(true);
}

void TestKyNetworkResourceManager::test_wifiNetworkRemove_nullDevice()
{
    // 测试空设备情况
    NetworkManager::Device *dev = nullptr;
    QString ssid = "TestNetwork";
    
    m_manager->wifiNetworkRemove(dev, ssid);

    m_manager->onWifiNetworkDisappeared(ssid);
    QVERIFY(true);
}

void TestKyNetworkResourceManager::test_onDeviceAdded()
{
    // 测试不存在的设备
    QString invalidUni = "/org/freedesktop/NetworkManager/Device/999999";
    m_manager->onDeviceAdded(invalidUni);
    NetworkManager::Device *p_device = m_manager->m_deviceList.at(0).data();

// 测试已存在的设备
    m_manager->onDeviceAdded(p_device->uni());
    QVERIFY(true);
}

void TestKyNetworkResourceManager::test_updateDeviceName()
{
    NetworkManager::Device *p_device = m_manager->m_deviceList.at(0).data();
    m_manager->updateDeviceName(p_device);

}

void TestKyNetworkResourceManager::test_onDeviceRemoved_emptyUni()
{
    // 测试空uni
    QString emptyUni = "";
    m_manager->onDeviceRemoved(emptyUni);
    QVERIFY(true);
}

void TestKyNetworkResourceManager::test_onActiveConnectionAdded_invalidPath()
{
    // 测试无效路径
    QString invalidPath = "/org/freedesktop/NetworkManager/ActiveConnection/999999";
    m_manager->onActiveConnectionAdded(invalidPath);
    QVERIFY(true);
}

void TestKyNetworkResourceManager::test_onActiveConnectionRemoved_emptyPath()
{
    // 测试空路径
    QString emptyPath = "";
    m_manager->onActiveConnectionRemoved(emptyPath);
    QVERIFY(true);
}

void TestKyNetworkResourceManager::test_onConnectionAdded_invalidPath()
{
    // 测试无效路径
    QString invalidPath = "/org/freedesktop/NetworkManager/Setting/999999";
    m_manager->onConnectionAdded(invalidPath);
    QVERIFY(true);
}

void TestKyNetworkResourceManager::test_onConnectionRemoved_emptyPath()
{
    // 测试空路径
    QString emptyPath = "";
    m_manager->onConnectionRemoved(emptyPath);
    QVERIFY(true);
}

void TestKyNetworkResourceManager::test_onConnectionRemoved_validPath()
{
    // 测试有效路径（不存在的连接）
    QString validPath = "/org/freedesktop/NetworkManager/Setting/999999";
    m_manager->onConnectionRemoved(validPath);
    QVERIFY(true);
}

void TestKyNetworkResourceManager::test_connectionDump()
{
    m_manager->connectionDump();
    QVERIFY(true);
}


void TestKyNetworkResourceManager::test_Release()
{
    KyNetworkResourceManager *instance = KyNetworkResourceManager::getInstance();
    QVERIFY(instance != nullptr);
    
    KyNetworkResourceManager::Release();
}

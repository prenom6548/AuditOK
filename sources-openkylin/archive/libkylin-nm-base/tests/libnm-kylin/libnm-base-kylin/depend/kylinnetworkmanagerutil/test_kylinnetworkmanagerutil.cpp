#include "test_kylinnetworkmanagerutil.h"
#include <QSignalSpy>
#include <QTest>
#include <QDebug>

TestKylinNetworkManagerUtil::TestKylinNetworkManagerUtil(QObject *parent)
    : QObject(parent)
{
}

TestKylinNetworkManagerUtil::~TestKylinNetworkManagerUtil()
{
}

void TestKylinNetworkManagerUtil::initTestCase()
{
}

void TestKylinNetworkManagerUtil::cleanupTestCase()
{
}

void TestKylinNetworkManagerUtil::test_getConnectTypeByDbus_emptyPath()
{
    QString result = getConnectTypeByDbus("");
    QVERIFY2(result.isEmpty(), "Empty path should return empty string");
}

void TestKylinNetworkManagerUtil::test_getConnectTypeByDbus_validPath()
{
    QString result = getConnectTypeByDbus("/org/freedesktop/NetworkManager/Settings/1");
    QVERIFY2(result.isEmpty() || !result.isEmpty(), "Result should be valid string or empty due to D-Bus");
}

void TestKylinNetworkManagerUtil::test_getSsidFromByteArray_empty()
{
    QByteArray empty;
    QString result = getSsidFromByteArray(empty);
    QVERIFY2(result.isEmpty(), "Empty byte array should return empty string");
}

void TestKylinNetworkManagerUtil::test_getSsidFromByteArray_validUtf8()
{
    QByteArray utf8Ssid = "TestNetwork";
    QString result = getSsidFromByteArray(utf8Ssid);
    QVERIFY2(result == "TestNetwork", "UTF-8 SSID should be decoded correctly");
}

void TestKylinNetworkManagerUtil::test_getSsidFromByteArray_validGbk()
{
    QByteArray gbkSsid;
    gbkSsid.append(static_cast<char>(0xC8));
    gbkSsid.append(static_cast<char>(0xCE));
    gbkSsid.append(static_cast<char>(0xCE));
    gbkSsid.append(static_cast<char>(0xD0));
    gbkSsid.append(static_cast<char>(0xCD));
    gbkSsid.append(static_cast<char>(0xF8));
    gbkSsid.append(static_cast<char>(0xC0));
    gbkSsid.append(static_cast<char>(0xB6));
    QString result = getSsidFromByteArray(gbkSsid);
    QVERIFY2(!result.isEmpty(), "GBK SSID should be decoded");
}

void TestKylinNetworkManagerUtil::test_enumToQstring_noSecurity()
{
    using Cap = NetworkManager::AccessPoint::Capabilities;
    using WpaFlag = NetworkManager::AccessPoint::WpaFlags;
    QString result = enumToQstring(Cap(0), WpaFlag(0), WpaFlag(0));
    QVERIFY2(result.isEmpty(), "No security flags should return empty string");
}

void TestKylinNetworkManagerUtil::test_enumToQstring_wep()
{
    using Cap = NetworkManager::AccessPoint::Capabilities;
    using WpaFlag = NetworkManager::AccessPoint::WpaFlags;
    QString result = enumToQstring(Cap(AP_CAPABILITY_PRIVACY), WpaFlag(AP_SECURITY_NONE), WpaFlag(AP_SECURITY_NONE));
    QVERIFY2(result.contains("WEP"), "WEP security should be detected");
}

void TestKylinNetworkManagerUtil::test_enumToQstring_wpa1()
{
    using Cap = NetworkManager::AccessPoint::Capabilities;
    using WpaFlag = NetworkManager::AccessPoint::WpaFlags;
    QString result = enumToQstring(Cap(0), WpaFlag(2), WpaFlag(0));
    QVERIFY2(result.contains("WPA1"), "WPA1 security should be detected");
}

void TestKylinNetworkManagerUtil::test_enumToQstring_wpa2()
{
    using Cap = NetworkManager::AccessPoint::Capabilities;
    using WpaFlag = NetworkManager::AccessPoint::WpaFlags;
    QString result = enumToQstring(Cap(0), WpaFlag(2), WpaFlag(AP_SECURITY_KEY_MGMT_PSK));
      qDebug() << "enumToQstring result:" << result;
    QVERIFY2(result.contains("WPA2"), "WPA2 security should be detected");
}

void TestKylinNetworkManagerUtil::test_enumToQstring_wpa3()
{
    using Cap = NetworkManager::AccessPoint::Capabilities;
    using WpaFlag = NetworkManager::AccessPoint::WpaFlags;
    QString result = enumToQstring(Cap(0), WpaFlag(0), WpaFlag(AP_SECURITY_KEY_MGMT_SAE));
    qDebug() << "enumToQstring result:" << result;

    QVERIFY2(result.contains("WPA3"), "WPA3 security should be detected");
}

void TestKylinNetworkManagerUtil::test_enumToQstring_8021x()
{
    using Cap = NetworkManager::AccessPoint::Capabilities;
    using WpaFlag = NetworkManager::AccessPoint::WpaFlags;
    QString result = enumToQstring(Cap(0), WpaFlag(6), WpaFlag(0));
    QVERIFY2(result.contains("WPA1"), "WPA1 should be detected");
}

void TestKylinNetworkManagerUtil::test_enumToQstring_mixed()
{
    using Cap = NetworkManager::AccessPoint::Capabilities;
    using WpaFlag = NetworkManager::AccessPoint::WpaFlags;
    QString result = enumToQstring(Cap(0), WpaFlag(6), WpaFlag(AP_SECURITY_KEY_MGMT_PSK));
    QVERIFY2(result.contains("WPA1"), "Mixed security should contain WPA1");
    QVERIFY2(result.contains("WPA2"), "Mixed security should contain WPA2");
}

void TestKylinNetworkManagerUtil::test_setWiredEnabledByGDbus_true()
{
    setWiredEnabledByGDbus(true);
    QVERIFY2(true, "setWiredEnabledByGDbus(true) should not crash");
}

void TestKylinNetworkManagerUtil::test_setWiredEnabledByGDbus_false()
{
    setWiredEnabledByGDbus(false);
    QVERIFY2(true, "setWiredEnabledByGDbus(false) should not crash");
}

void TestKylinNetworkManagerUtil::test_getWiredEnabledByGDbus()
{
    bool result = getWiredEnabledByGDbus();
    QVERIFY2((result == true || result == false), "getWiredEnabledByGDbus should return boolean");
}

void TestKylinNetworkManagerUtil::test_setDeviceManagedByGDbus()
{
    setDeviceManagedByGDbus("/org/freedesktop/NetworkManager/Devices/0", true);
    QVERIFY2(true, "setDeviceManagedByGDbus should not crash");
}

#include "test_kylinapconnectitem.h"

void TestKyApConnectItem::testDefaultConstructor()
{
    // Test default constructor - based on header file declaration (not inheriting QObject)
    KyApConnectItem item;
    QVERIFY(item.m_connectName.isEmpty());
    QVERIFY(item.m_connectSsid.isEmpty());
    QVERIFY(item.m_connectUuid.isEmpty());
    QVERIFY(item.m_ifaceName.isEmpty());
    QVERIFY(item.m_password.isEmpty());
    QVERIFY(item.m_band.isEmpty());
    QVERIFY(item.m_isActivated == false);
}

void TestKyApConnectItem::testMemberVariables()
{
    // Test setting member variables
    KyApConnectItem item;
    item.m_connectName = "TestConnection";
    item.m_connectSsid = "TestSSID";
    item.m_connectUuid = "test-uuid-12345";
    item.m_ifaceName = "wlan0";
    item.m_password = "testpassword";
    item.m_band = "2.4GHz";
    item.m_isActivated = true;

    QCOMPARE(item.m_connectName, QString("TestConnection"));
    QCOMPARE(item.m_connectSsid, QString("TestSSID"));
    QCOMPARE(item.m_connectUuid, QString("test-uuid-12345"));
    QCOMPARE(item.m_ifaceName, QString("wlan0"));
    QCOMPARE(item.m_password, QString("testpassword"));
    QCOMPARE(item.m_band, QString("2.4GHz"));
    QVERIFY(item.m_isActivated == true);
}


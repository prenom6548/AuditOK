#include "test_kywirelessnetitem.h"

void TestKyWirelessNetItem::SetUp()
{
    m_testItem = new KyWirelessNetItem();
}

void TestKyWirelessNetItem::TearDown()
{
    delete m_testItem;
    m_testItem = nullptr;
}

TEST_F(TestKyWirelessNetItem, TestDefaultConstructor)
{
    EXPECT_EQ(m_testItem->m_NetSsid, "");
    EXPECT_EQ(m_testItem->m_connectUuid, "");
    EXPECT_EQ(m_testItem->m_dbusPath, "");
    EXPECT_EQ(m_testItem->m_connectSecuType, KySecuType::NONE);
    EXPECT_EQ(m_testItem->m_isApConnection, false);
    EXPECT_EQ(m_testItem->m_secuType, "");
    EXPECT_EQ(m_testItem->m_signalStrength, 0);
    EXPECT_EQ(m_testItem->m_frequency, 0);
    EXPECT_EQ(m_testItem->m_isMix, false);
}

TEST_F(TestKyWirelessNetItem, TestMemberInitialization)
{
    m_testItem->m_NetSsid = "TestSSID";
    m_testItem->m_connectUuid = "test-uuid-12345";
    m_testItem->m_dbusPath = "/org/freedesktop/NetworkManager/AccessPoint/1";
    m_testItem->m_connectSecuType = KySecuType::WPA_AND_WPA2_PERSONAL;
    m_testItem->m_isApConnection = true;
    m_testItem->m_secuType = "wpa2";
    m_testItem->m_signalStrength = 75;
    m_testItem->m_frequency = 2437;
    m_testItem->m_isMix = true;

    EXPECT_EQ(m_testItem->m_NetSsid, "TestSSID");
    EXPECT_EQ(m_testItem->m_connectUuid, "test-uuid-12345");
    EXPECT_EQ(m_testItem->m_dbusPath, "/org/freedesktop/NetworkManager/AccessPoint/1");
    EXPECT_EQ(m_testItem->m_connectSecuType, KySecuType::WPA_AND_WPA2_PERSONAL);
    EXPECT_EQ(m_testItem->m_isApConnection, true);
    EXPECT_EQ(m_testItem->m_secuType, "wpa2");
    EXPECT_EQ(m_testItem->m_signalStrength, 75);
    EXPECT_EQ(m_testItem->m_frequency, 2437);
    EXPECT_EQ(m_testItem->m_isMix, true);
}

TEST_F(TestKyWirelessNetItem, TestDifferentSecuTypes)
{
    m_testItem->m_connectSecuType = KySecuType::UNKNOWN;
    EXPECT_EQ(m_testItem->m_connectSecuType, KySecuType::UNKNOWN);

    m_testItem->m_connectSecuType = KySecuType::WPA_AND_WPA2_ENTERPRISE;
    EXPECT_EQ(m_testItem->m_connectSecuType, KySecuType::WPA_AND_WPA2_ENTERPRISE);

    m_testItem->m_connectSecuType = KySecuType::WPA3_PERSONAL;
    EXPECT_EQ(m_testItem->m_connectSecuType, KySecuType::WPA3_PERSONAL);
}

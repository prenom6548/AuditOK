#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>
#include <QDBusArgument>
#include "../../../ukui-session/ukuilockinfo.h"

#include "stubext.h"
using namespace stub_ext;

class UkuilockinfoInfoTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_ukuilockinfo = new Ukuilockinfo();
    }

    static void TearDownTestSuite()
    {
       // delete m_ukuilockinfo;
        //m_ukuilockinfo = nullptr;
    }

    static Ukuilockinfo *m_ukuilockinfo;
};

Ukuilockinfo *UkuilockinfoInfoTest::m_ukuilockinfo = nullptr;


TEST_F(UkuilockinfoInfoTest, ListInhibitorInfo)
{
    QVector<InhibitInfo::InhibitorInfo> result = m_ukuilockinfo->listInhibitorInfo(Ukuilockinfo::InhibitorType::logout);
    EXPECT_TRUE(result.isEmpty());
}


TEST_F(UkuilockinfoInfoTest, GetCfgValue)
{

        // switchuserBtn,
        // hibernateBtn,
        // suspendBtn,
        // lockscreenBtn,
        // logoutBtn,
        // rebootBtn,
        // shutdownBtn
    bool result1 = m_ukuilockinfo->getCfgValue(Ukuilockinfo::buttonType::switchuserBtn);
    EXPECT_TRUE(result1);

    bool result2 = m_ukuilockinfo->getCfgValue(Ukuilockinfo::buttonType::hibernateBtn);
    EXPECT_TRUE(result2);

    bool result3 = m_ukuilockinfo->getCfgValue(Ukuilockinfo::buttonType::suspendBtn);
    EXPECT_TRUE(result3);

    bool result4 = m_ukuilockinfo->getCfgValue(Ukuilockinfo::buttonType::lockscreenBtn);
    EXPECT_TRUE(result4);

    bool result5 = m_ukuilockinfo->getCfgValue(Ukuilockinfo::buttonType::logoutBtn);
    EXPECT_TRUE(result5);

    bool result6 = m_ukuilockinfo->getCfgValue(Ukuilockinfo::buttonType::rebootBtn);
    EXPECT_TRUE(result6);

    bool result7 = m_ukuilockinfo->getCfgValue(Ukuilockinfo::buttonType::shutdownBtn);
    EXPECT_TRUE(result7);
}

TEST_F(UkuilockinfoInfoTest, GetSystemdInhibitor)
{
    QVector<InhibitInfo::InhibitorInfo> result;
    m_ukuilockinfo->getSystemdInhibitor(QString("sleep"),result);
    
    EXPECT_TRUE(result.isEmpty());
}


TEST_F(UkuilockinfoInfoTest, operator1)
{
    QDBusArgument argument;
    SystemdInhibitor::Inhibitor testInhibitor;
    // 方法 2：逐个提取成员并验证
    QString action, name, reason, mode;
    uint uid, pid;

    argument.beginStructure();
    argument >> action >> name >> reason >> mode >> uid >> pid;
    argument.endStructure();

    // 验证提取的值是否与原始值一致
    EXPECT_EQ(action, testInhibitor.action);
    EXPECT_EQ(name, testInhibitor.name);
    EXPECT_EQ(reason, testInhibitor.reason);
    EXPECT_EQ(mode, testInhibitor.mode);
    EXPECT_EQ(uid, testInhibitor.uid);
    EXPECT_EQ(pid, testInhibitor.pid);

}

TEST_F(UkuilockinfoInfoTest, GetLogoutInhibitor)
{
    QVector<InhibitInfo::InhibitorInfo> result;
    m_ukuilockinfo->getLogoutInhibitor(result);
    
    EXPECT_TRUE(result.isEmpty());
}

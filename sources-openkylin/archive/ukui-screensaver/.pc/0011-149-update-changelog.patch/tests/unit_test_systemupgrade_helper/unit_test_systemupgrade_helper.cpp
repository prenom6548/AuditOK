#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>

#include "../../src/dbusifs/systemupgradehelper.h"

#include "stubext.h"

using namespace stub_ext;

class SystemUpgradeHelperTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pSystemUpgradeHelperDbus = new SystemUpgradeHelper();
    }

    static void TearDownTestSuite()
    {
        delete m_pSystemUpgradeHelperDbus;
        m_pSystemUpgradeHelperDbus = nullptr;
    }

    static SystemUpgradeHelper *m_pSystemUpgradeHelperDbus;
};

SystemUpgradeHelper *SystemUpgradeHelperTest::m_pSystemUpgradeHelperDbus = nullptr;

TEST_F(SystemUpgradeHelperTest, checkSystemUpgrade)
{
    bool value = m_pSystemUpgradeHelperDbus->checkSystemUpgrade();
    ASSERT_EQ(value, false);
}

TEST_F(SystemUpgradeHelperTest, doUpgradeThenRboot)
{
    bool value = m_pSystemUpgradeHelperDbus->doUpgradeThenRboot();
    ASSERT_EQ(value, false);
}

TEST_F(SystemUpgradeHelperTest, doUpgradeThenShutdown)
{
    bool value = m_pSystemUpgradeHelperDbus->doUpgradeThenShutdown();
    ASSERT_EQ(value, false);
}

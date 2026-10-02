#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>

#include "../../src/dbusifs/login1helper.h"

#include "stubext.h"
using namespace stub_ext;

class Login1HelperTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pLogin1HelperDbus = new Login1Helper();
    }

    static void TearDownTestSuite()
    {
        delete m_pLogin1HelperDbus;
        m_pLogin1HelperDbus = nullptr;
    }

    static Login1Helper *m_pLogin1HelperDbus;
};

Login1Helper *Login1HelperTest::m_pLogin1HelperDbus = nullptr;

TEST_F(Login1HelperTest, isSessionActive)
{
    bool value = m_pLogin1HelperDbus->isSessionActive();
    ASSERT_EQ(value, true);
}

TEST_F(Login1HelperTest, onPrepareForSleep)
{
    bool oldSleep = m_pLogin1HelperDbus->m_isSleeping;
    m_pLogin1HelperDbus->onPrepareForSleep(true);
    m_pLogin1HelperDbus->onHibitedWatcherMessage();
    m_pLogin1HelperDbus->onPrepareForSleep(oldSleep);
    ASSERT_EQ(m_pLogin1HelperDbus->m_isSleeping, true);
}

TEST_F(Login1HelperTest, setPowerManager)
{
    m_pLogin1HelperDbus->setPowerManager("test");
    m_pLogin1HelperDbus->Unlock();
}

TEST_F(Login1HelperTest, getCanPowerManager)
{
    bool value = m_pLogin1HelperDbus->getCanPowerManager("CanReboot");
    ASSERT_EQ(value, true);
}

TEST_F(Login1HelperTest, isCanHibernate)
{
    bool value = m_pLogin1HelperDbus->isCanHibernate();
    ASSERT_EQ(value, true);
}

TEST_F(Login1HelperTest, isCanSuspend)
{
    bool value = m_pLogin1HelperDbus->isCanSuspend();
    ASSERT_EQ(value, true);
}

TEST_F(Login1HelperTest, isCanReboot)
{
    bool value = m_pLogin1HelperDbus->isCanReboot();
    ASSERT_EQ(value, true);
}

TEST_F(Login1HelperTest, isCanPowerOff)
{
    bool value = m_pLogin1HelperDbus->isCanPowerOff();
    ASSERT_EQ(value, true);
}

TEST_F(Login1HelperTest, canAction)
{
    bool value = m_pLogin1HelperDbus->canAction(PowerHibernate);
    bool value1 = m_pLogin1HelperDbus->canAction(PowerSuspend);
    bool value2 = m_pLogin1HelperDbus->canAction(PowerMonitorOff);
    bool value3 = m_pLogin1HelperDbus->canAction(PowerLogout);
    bool value4 = m_pLogin1HelperDbus->canAction(PowerReboot);
    bool value5 = m_pLogin1HelperDbus->canAction(PowerShutdown);
    ASSERT_EQ(value, true);
    ASSERT_EQ(value1, true);
    ASSERT_EQ(value2, true);
    ASSERT_EQ(value3, true);
    ASSERT_EQ(value4, true);
    ASSERT_EQ(value5, true);
}



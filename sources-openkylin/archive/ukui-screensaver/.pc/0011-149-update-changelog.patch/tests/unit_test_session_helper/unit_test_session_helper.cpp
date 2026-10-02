#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>

#include "../../src/dbusifs/sessionhelper.h"
#include "../../src/dbusifs/login1helper.h"

#include "stubext.h"

using namespace stub_ext;

class SessionHelperTest : public testing::Test, public QObject
{
protected:
    static void SetUpTestSuite()
    {
        m_login1Helper = QSharedPointer<Login1Helper>(new Login1Helper(nullptr));
        m_pSessionHelperDbus = new SessionHelper(m_login1Helper);
    }

    static void TearDownTestSuite()
    {
        delete m_pSessionHelperDbus;
        m_pSessionHelperDbus = nullptr;
    }

    static SessionHelper *m_pSessionHelperDbus;
    static QSharedPointer<Login1Helper> m_login1Helper;
};

SessionHelper *SessionHelperTest::m_pSessionHelperDbus = nullptr;
QSharedPointer<Login1Helper> SessionHelperTest::m_login1Helper = nullptr;

TEST_F(SessionHelperTest, canAction)
{
    bool value = m_pSessionHelperDbus->canAction(PowerHibernate);
    bool value1 = m_pSessionHelperDbus->canAction(PowerSuspend);
    bool value2 = m_pSessionHelperDbus->canAction(PowerMonitorOff);
    bool value3 = m_pSessionHelperDbus->canAction(PowerLogout);
    bool value4 = m_pSessionHelperDbus->canAction(PowerReboot);
    bool value5 = m_pSessionHelperDbus->canAction(PowerShutdown);
    bool value6 = m_pSessionHelperDbus->canAction(PowerSwitchUser);
    //    bool value7 = m_pSessionHelperDbus->canAction(TestAction);
    ASSERT_EQ(value, true);
    ASSERT_EQ(value1, true);
    ASSERT_EQ(value2, true);
    ASSERT_EQ(value3, true);
    ASSERT_EQ(value4, true);
    ASSERT_EQ(value5, true);
    ASSERT_EQ(value6, true);
}

TEST_F(SessionHelperTest, doAction)
{
    bool value = m_pSessionHelperDbus->doAction("Suspend");
    ASSERT_EQ(value, true);
}

TEST_F(SessionHelperTest, getLockCheckStatus)
{
    QStringList value = m_pSessionHelperDbus->getLockCheckStatus("shutdown");
    ASSERT_EQ(value.isEmpty(), true);
}

TEST_F(SessionHelperTest, playShutdownMusic)
{
    bool value = m_pSessionHelperDbus->playShutdownMusic("Logout");
    m_pSessionHelperDbus->playShutdownMusic("Reboot");
    m_pSessionHelperDbus->playShutdownMusic("SwitchUser");
    ASSERT_EQ(value, true);
}

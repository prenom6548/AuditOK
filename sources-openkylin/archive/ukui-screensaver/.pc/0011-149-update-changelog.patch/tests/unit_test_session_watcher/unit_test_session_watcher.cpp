#include <gtest/gtest.h>

#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>
#include "../../src/lock-backend/sessionwatcher.h"

#include "stubext.h"
using namespace stub_ext;

class SessionWatcherTest : public testing::Test
{
protected:
    QSharedPointer<GSettingsHelper> m_pGSettingsHelperDbus;
    QSharedPointer<SessionWatcher> m_pSessionWatcherDbus;

    void SetUp() override
    {
        m_pGSettingsHelperDbus = QSharedPointer<GSettingsHelper>(new GSettingsHelper());
        m_pSessionWatcherDbus = QSharedPointer<SessionWatcher>(new SessionWatcher(m_pGSettingsHelperDbus));
    }

    void TearDown() override
    {
        m_pSessionWatcherDbus.reset();
        m_pGSettingsHelperDbus.reset();
    }
};

TEST_F(SessionWatcherTest, GetCurUserName)
{
    m_pSessionWatcherDbus->onStatusChanged(0);
    m_pSessionWatcherDbus->onStatusChanged(1);
    m_pSessionWatcherDbus->onStatusChanged(2);
    m_pSessionWatcherDbus->onStatusChanged(3);
}

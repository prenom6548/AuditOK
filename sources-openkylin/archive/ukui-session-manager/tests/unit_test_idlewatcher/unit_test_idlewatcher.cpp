#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>

#include "../../../ukui-session/idlewatcher.h"

#include "stubext.h"
using namespace stub_ext;

class IdleWatcherTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_idlewatcher = new IdleWatcher(60);
    }

    static void TearDownTestSuite()
    {
        delete m_idlewatcher;
        m_idlewatcher = nullptr;
    }

    static IdleWatcher *m_idlewatcher;
};

IdleWatcher *IdleWatcherTest::m_idlewatcher = nullptr;

TEST_F(IdleWatcherTest, isWemeetappRunning)
{
    bool result = m_idlewatcher->isWemeetappRunning();
    EXPECT_FALSE(result);
}

TEST_F(IdleWatcherTest, reset)
{
    m_idlewatcher->reset(1);
    EXPECT_EQ(m_idlewatcher->mSecsidle,1);
}

TEST_F(IdleWatcherTest, ResumingFromIdle)
{
    bool ok = true;
    m_idlewatcher->resumingFromIdle();
    EXPECT_TRUE(ok);
}

TEST_F(IdleWatcherTest, WeakupFromSleep)
{
    bool ok = true;
    m_idlewatcher->weakupFromSleep(false);
    EXPECT_TRUE(ok);
}

TEST_F(IdleWatcherTest, TimeoutReached)
{
    m_idlewatcher->mSecsidle = 60;
    m_idlewatcher->timeoutReached(1,60);
    EXPECT_EQ(m_idlewatcher->mSecsidle,60);
}

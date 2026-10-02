#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>

#include "../../../ukui-session/sessionapplication.h"

#include "stubext.h"
using namespace stub_ext;

class SessionApplicationTest: public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        int argc = 2;
        char* argv[] = {"test", "test"};
        m_sessionapplication = new SessionApplication(argc,argv);
    }

    static void TearDownTestSuite()
    {
        delete m_sessionapplication;
        m_sessionapplication = nullptr;
    }

    static SessionApplication *m_sessionapplication;
};

SessionApplication *SessionApplicationTest::m_sessionapplication = nullptr;


TEST_F(SessionApplicationTest, InitialEnvironment)
{
    //m_sessionapplication->initialEnvironment();
    //EXPECT_TRUE(m_sessionapplication->isok);
}






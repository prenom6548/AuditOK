#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>

#include "../../src/dbusifs/freedesktophelper.h"

#include "stubext.h"
using namespace stub_ext;

class FreedesktopHelperTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pFreedesktopHelperDbus = new FreedesktopHelper();
    }

    static void TearDownTestSuite()
    {
        delete m_pFreedesktopHelperDbus;
        m_pFreedesktopHelperDbus = nullptr;
    }

    static FreedesktopHelper *m_pFreedesktopHelperDbus;
};

FreedesktopHelper *FreedesktopHelperTest::m_pFreedesktopHelperDbus = nullptr;

TEST_F(FreedesktopHelperTest, NameHasOwner)
{
    bool value = m_pFreedesktopHelperDbus->NameHasOwner("org.ukui.Biometric");
    ASSERT_EQ(value, false);
}

TEST_F(FreedesktopHelperTest, isServiceActivable)
{
    bool value = m_pFreedesktopHelperDbus->isServiceActivable("org.ukui.UniauthBackend");
    ASSERT_EQ(value, false);
}

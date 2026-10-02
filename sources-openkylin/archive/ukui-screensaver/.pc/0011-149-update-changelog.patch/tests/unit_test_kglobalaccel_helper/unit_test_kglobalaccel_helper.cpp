#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>

#include "../../src/dbusifs/kglobalaccelhelper.h"

#include "stubext.h"
using namespace stub_ext;

class KglobalAccelHelperTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pKglobalAccelHelperDbus = new KglobalAccelHelper();
    }

    static void TearDownTestSuite()
    {
        delete m_pKglobalAccelHelperDbus;
        m_pKglobalAccelHelperDbus = nullptr;
    }

    static KglobalAccelHelper *m_pKglobalAccelHelperDbus;
};

KglobalAccelHelper *KglobalAccelHelperTest::m_pKglobalAccelHelperDbus = nullptr;

TEST_F(KglobalAccelHelperTest, blockShortcut)
{
    bool oldValue = m_pKglobalAccelHelperDbus->blockShortcut(true);
    ASSERT_EQ(oldValue, true);

    bool newValue = m_pKglobalAccelHelperDbus->blockShortcut(false);
    ASSERT_EQ(newValue, true);
}


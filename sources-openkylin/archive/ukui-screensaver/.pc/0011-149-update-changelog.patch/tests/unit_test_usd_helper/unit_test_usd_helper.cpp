#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>

#include "../../src/dbusifs/usdhelper.h"

#include "stubext.h"
using namespace stub_ext;

class UsdHelperTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pUsdHelperDbus = new UsdHelper();
    }

    static void TearDownTestSuite()
    {
        delete m_pUsdHelperDbus;
        m_pUsdHelperDbus = nullptr;
    }

    static UsdHelper *m_pUsdHelperDbus;
};

UsdHelper *UsdHelperTest::m_pUsdHelperDbus = nullptr;

TEST_F(UsdHelperTest, usdExternalDoAction)
{
    bool value = m_pUsdHelperDbus->usdExternalDoAction(-1);
    ASSERT_EQ(value, false);
}

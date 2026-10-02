#include <gtest/gtest.h>

#include "../../src/common/displayservice.h"

class DisplayServiceTest : public testing::Test
{
protected:
//    static void SetUpTestSuite()
//    {
//        m_pGSettingsHelperDbus = new GSettingsHelper();
//    }

//    static void TearDownTestSuite()
//    {
//        delete m_pGSettingsHelperDbus;
//        m_pGSettingsHelperDbus = nullptr;
//    }

//    static GSettingsHelper *m_pGSettingsHelperDbus;
};

//GSettingsHelper *DisplayServiceTest::m_pGSettingsHelperDbus = nullptr;

// 测试 switchDisplayMode 函数
TEST_F(DisplayServiceTest, SwitchDisplayMode) {
    // 测试 DISPLAY_MODE_ORI
    bool value = DisplayService::instance()->switchDisplayMode(DISPLAY_MODE_ORI);
    EXPECT_EQ(value, false);

    // 测试 DISPLAY_MODE_CLONE
    bool value1 = DisplayService::instance()->switchDisplayMode(DISPLAY_MODE_CLONE);
    EXPECT_EQ(value1, false);

    // 测试 DISPLAY_MODE_EXTEND
    bool value2 = DisplayService::instance()->switchDisplayMode(DISPLAY_MODE_EXTEND);
    EXPECT_EQ(value2, false);

    // 测试 DISPLAY_MODE_ONLY_OUT
    bool value3 = DisplayService::instance()->switchDisplayMode(DISPLAY_MODE_ONLY_OUT);
    EXPECT_EQ(value3, false);
}


// 测试 setOneDisplayMode 函数
TEST_F(DisplayServiceTest, SetOneDisplayMode) {
    // 测试使用 xrandr
    DisplayService::instance()->setOneDisplayMode();
}


// 测试 setCurUserName 函数
TEST_F(DisplayServiceTest, SetCurUserName) {
    DisplayService::instance()->setCurUserName(getenv("USER"));
}


// 测试 isSaveParamInUsed 函数
TEST_F(DisplayServiceTest, IsSaveParamInUsed) {
    EXPECT_TRUE(DisplayService::instance()->isSaveParamInUsed());

    EXPECT_FALSE(DisplayService::instance()->isSaveParamInUsed());
}


// 测试 isJJW7200 函数
TEST_F(DisplayServiceTest, IsJJW7200) {
    int value = DisplayService::instance()->isJJW7200();
    EXPECT_EQ(value, 0);
}

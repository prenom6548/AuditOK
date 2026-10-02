#include "gtest/gtest.h"
#include "../registeredQDbus/sysdbusregister.h"

class SysDbusRegisterTest : public ::testing::Test {
protected:
    void SetUp() override {
        register_ = new SysdbusRegister();
    }

    void TearDown() override {
        delete register_;
    }

    SysdbusRegister* register_{nullptr};
};
// 用户管理测试
TEST_F(SysDbusRegisterTest, UserManagementTest) {
    QString testUser = "testUser";
    EXPECT_NO_THROW(register_->UserLogin(testUser));
    EXPECT_NO_THROW(register_->UserActive(testUser));
    EXPECT_NO_THROW(register_->UserLogout(testUser));
}

// 亮度控制测试
TEST_F(SysDbusRegisterTest, BrightnessControlTest) {
    EXPECT_NO_THROW({
        bool canSetBrightness = register_->CanSetBrightness();
        if (canSetBrightness) {
            int maxBrightness = register_->GetMaxBrightness();
            EXPECT_GT(maxBrightness, 0);

            int oldBrightness = register_->GetBrightnessPercent();
            register_->SetBrightnessPercent(50);
            EXPECT_EQ(register_->GetBrightnessPercent(), 50);
            register_->SetBrightnessPercent(oldBrightness);
        }
    });
    
    EXPECT_NO_THROW(register_->SetReduceBacklightSwitch(true));
}

// 电源策略测试
TEST_F(SysDbusRegisterTest, PowerPolicyTest) {
    int oldCpuPolicy = register_->GetCpuPolicy();
    
    EXPECT_NO_THROW({
        register_->SetPcPolicy(1);
        register_->SetCpuPolicy(1);
        register_->SetGpuPolicy(1);
        register_->SetAudioPolicy(1);
        register_->SetAspmPolicy(1);
    });
    
    EXPECT_EQ(register_->GetCpuPolicy(), 1);
    register_->SetCpuPolicy(oldCpuPolicy);
}
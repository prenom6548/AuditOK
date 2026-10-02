#include "gtest/gtest.h"
#include "../registeredQDbus/hardware-power-control/hardwarepowercontrol.h"
class HardwarePowerControlTest : public ::testing::Test {
protected:
    void SetUp() override {
        hwControl = new HardwarePowerControl();
    }
    void TearDown() override {
        delete hwControl;
    }

    HardwarePowerControl* hwControl{nullptr};
};

// GPU频率模式测试
TEST_F(HardwarePowerControlTest, GpuFrequencyTest) {
    // 测试有效值
    for(int policy = 0; policy <= 2; policy++) {
        EXPECT_NO_THROW(hwControl->setGpuFrequencyMode(policy));
    }
    
    // 测试无效值
    EXPECT_NO_THROW({
        hwControl->setGpuFrequencyMode(-1);
        hwControl->setGpuFrequencyMode(3);
    });
}

// 音频模式测试
TEST_F(HardwarePowerControlTest, AudioModeTest) {
    // 测试有效值
    for(int policy = 0; policy <= 2; policy++) {
        EXPECT_NO_THROW(hwControl->setAudioMode(policy));
    }
    
    // 测试无效值
    EXPECT_NO_THROW({
        hwControl->setAudioMode(-1);
        hwControl->setAudioMode(3);
    });
}

// PCIE ASPM模式测试
TEST_F(HardwarePowerControlTest, PcieAspmTest) {
    // 测试有效值
    for(int policy = 0; policy <= 2; policy++) {
        EXPECT_NO_THROW(hwControl->setPcieAspmMode(policy));
    }
    
    // 测试无效值
    EXPECT_NO_THROW({
        hwControl->setPcieAspmMode(-1);
        hwControl->setPcieAspmMode(3);
    });
}
#include "gtest/gtest.h"
#include "../registeredQDbus/cpu-control/upmcpucontrol.cpp"


class UpmCpuControlTest : public ::testing::Test {
protected:
    void SetUp() override {
        cpuControl = new UpmCpuControl();
    }

    void TearDown() override {
        delete cpuControl;
    }

    UpmCpuControl* cpuControl{nullptr};
};

// CPU核心数量测试
TEST_F(UpmCpuControlTest, CpuCoreNumTest) {
    EXPECT_GT(cpuControl->getCpuCoreNum(), 0);
}

// CPU核心策略测试
TEST_F(UpmCpuControlTest, CpuCorePolicyTest) {
    int coreNum = cpuControl->getCpuCoreNum();
    for(int i = 0; i < coreNum; i++) {
        EXPECT_NO_THROW({
            cpuControl->setCorePolicy(i, 1);
            cpuControl->setCoreMaxFreq(i, 2000000);
            int defaultFreq = cpuControl->getDefaultCoreMaxFreq(i);
            EXPECT_GT(defaultFreq, 0);
            cpuControl->setCoreDefaultMaxFreq(i);
        });
    }
}

// 全局CPU策略测试
TEST_F(UpmCpuControlTest, AllCorePolicyTest) {
    EXPECT_NO_THROW({
        cpuControl->setAllCorePolicy(1);
        cpuControl->setAllCoreMaxFreq(2000000);
        cpuControl->setAllCoreDefaultMaxFreq();
    });
}

// CPU占用参数配置测试
TEST_F(UpmCpuControlTest, CpuOccupyConfigTest) {
    EXPECT_NO_THROW(cpuControl->configCpuOccupyParameter(
        1,      // cpuPolicy
        100,    // cpuSamplingInterval
        50,     // offlineCpuOccupy
        80,     // onlineCpuOccupy
        1       // reserveCoreNum
    ));
}
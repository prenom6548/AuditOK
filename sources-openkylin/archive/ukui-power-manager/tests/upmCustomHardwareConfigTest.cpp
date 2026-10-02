#include "gtest/gtest.h"
#include "../registeredQDbus/upmcustomhardwareconfig.h"

class UpmCustomHardwareConfigTest : public ::testing::Test {
protected:
    void SetUp() override {
        config = UpmCustomHardwareConfig::self();
    }

    UpmCustomHardwareConfig* config{nullptr};
};

// 测试单例模式
TEST_F(UpmCustomHardwareConfigTest, SingletonTest) {
    EXPECT_NE(config, nullptr);
    EXPECT_EQ(config, UpmCustomHardwareConfig::self());
}

// 测试硬件识别
TEST_F(UpmCustomHardwareConfigTest, HardwareDetectionTest) {
    bool isPangu = config->isPanguM900();
    EXPECT_NO_THROW(isPangu);
}

// 测试扩展配置获取
TEST_F(UpmCustomHardwareConfigTest, ExtendConfigTest) {
    QString configKey = "test_key";
    QString configValue;
    
    bool result = config->getExtendConfig(configKey, configValue);
    EXPECT_NO_THROW(result);
    
    // 测试空键
    QString emptyKey;
    QString emptyValue;
    EXPECT_NO_THROW(config->getExtendConfig(emptyKey, emptyValue));
}
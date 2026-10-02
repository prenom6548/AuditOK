#include <gtest/gtest.h>
#include "common/usd_base_class.h"

class UsdBaseClassTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化测试环境
    }

    void TearDown() override {
        // 清理测试环境
    }
};

TEST_F(UsdBaseClassTest, IsTablet) {
    EXPECT_FALSE(UsdBaseClass::isTablet());
}

TEST_F(UsdBaseClassTest, IsTableWithDBus) {
    EXPECT_FALSE(UsdBaseClass::isTableWithDBus());
}

TEST_F(UsdBaseClassTest, IsMasterSP1) {
    EXPECT_FALSE(UsdBaseClass::isMasterSP1());
}

TEST_F(UsdBaseClassTest, Is9X0) {
    EXPECT_FALSE(UsdBaseClass::is9X0());
}

TEST_F(UsdBaseClassTest, IsWayland) {
    EXPECT_FALSE(UsdBaseClass::isWayland());
}

TEST_F(UsdBaseClassTest, IsWlcom) {
    EXPECT_FALSE(UsdBaseClass::isWlcom());
}

TEST_F(UsdBaseClassTest, IsXcb) {
    EXPECT_FALSE(UsdBaseClass::isXcb());
}

TEST_F(UsdBaseClassTest, IsNotebook) {
    EXPECT_FALSE(UsdBaseClass::isNotebook());
}

TEST_F(UsdBaseClassTest, IsUseXEventAsShutKey) {
    EXPECT_TRUE(UsdBaseClass::isUseXEventAsShutKey());
}

TEST_F(UsdBaseClassTest, ReadPowerOffConfig) {
    EXPECT_TRUE(UsdBaseClass::readPowerOffConfig());
}

TEST_F(UsdBaseClassTest, IsPowerOff) {
    EXPECT_FALSE(UsdBaseClass::isPowerOff());
}

TEST_F(UsdBaseClassTest, IsJJW7200) {
    EXPECT_FALSE(UsdBaseClass::isJJW7200());
}

TEST_F(UsdBaseClassTest, GetDPI) {
    EXPECT_EQ(UsdBaseClass::getDPI(), 96);
}

TEST_F(UsdBaseClassTest, GetScaleWithSize) {
    // 根据实际实现调整预期值
    EXPECT_DOUBLE_EQ(UsdBaseClass::getScaleWithSize(100, 100, 1920, 1080), 0.875);
}

TEST_F(UsdBaseClassTest, GetScale) {
    EXPECT_DOUBLE_EQ(UsdBaseClass::getScale(1.0), 0.5);
}

TEST_F(UsdBaseClassTest, GetScoreScale) {
    EXPECT_DOUBLE_EQ(UsdBaseClass::getScoreScale(1.0), 1.0);
}

TEST_F(UsdBaseClassTest, WriteUserConfigToLightDM) {
    UsdBaseClass::writeUserConfigToLightDM("group", "key", "value");
    EXPECT_EQ(UsdBaseClass::readUserConfigToLightDM("group", "key").toString(), "value");
}

TEST_F(UsdBaseClassTest, IsWaylandWithKscreen) {
    EXPECT_FALSE(UsdBaseClass::isWaylandWithKscreen());
}

TEST_F(UsdBaseClassTest, BrightnessControlByHardware) {
    int step = 0;
    EXPECT_FALSE(UsdBaseClass::brightnessControlByHardware(step));
}

TEST_F(UsdBaseClassTest, FlightModeControlByHardware) {
    int mode = 0;
    EXPECT_TRUE(UsdBaseClass::flightModeControlByHardware(mode));
}

TEST_F(UsdBaseClassTest, TouchpadControlByHardware) {
    int state = 0;
    EXPECT_FALSE(UsdBaseClass::touchpadControlByHardware(state));
}

TEST_F(UsdBaseClassTest, PowerModeControlByHardware) {
    int mode = 0;
    EXPECT_FALSE(UsdBaseClass::powerModeControlByHardware(mode));
}

TEST_F(UsdBaseClassTest, ReadInfoFromFile) {
    // 确保文件路径正确
    EXPECT_FALSE(UsdBaseClass::readInfoFromFile("/path/to/nonexistent/file").isValid());
}

TEST_F(UsdBaseClassTest, ReadHashFromFile) {
    EXPECT_EQ(UsdBaseClass::readHashFromFile("/path/to/nonexistent/file"), "false");
}

TEST_F(UsdBaseClassTest, IsVirt) {
    EXPECT_FALSE(UsdBaseClass::isVirt());
}

TEST_F(UsdBaseClassTest, GetConnectOutputCount) {
    EXPECT_EQ(UsdBaseClass::getConnectOutputCount(), 0);
}

TEST_F(UsdBaseClassTest, GetConnectOutputHash) {
    // 根据实际实现调整预期值
    EXPECT_EQ(UsdBaseClass::getConnectOutputHash(), "");
}

TEST_F(UsdBaseClassTest, GetOutputHash) {
    EXPECT_EQ(UsdBaseClass::getOutputHash("name"), "");
}

TEST_F(UsdBaseClassTest, GetConnectOutputHashWithPort) {
    // 根据实际实现调整预期值
    EXPECT_EQ(UsdBaseClass::getConnectOutputHashWithPort(), "");
}

TEST_F(UsdBaseClassTest, ParseVender) {
    QByteArray rawData;
    EXPECT_EQ(UsdBaseClass::parseVender(rawData), "");
}

TEST_F(UsdBaseClassTest, GetOutputVender) {
    EXPECT_EQ(UsdBaseClass::getOutputVender("outputTypeName"), "");
}

TEST_F(UsdBaseClassTest, WriteGlobalConfig) {
    QByteArray info;
    EXPECT_FALSE(UsdBaseClass::writeGlobalConfig("path", "fileName", info));
}

TEST_F(UsdBaseClassTest, ReadGlobalConfig) {
    EXPECT_EQ(UsdBaseClass::readGlobalConfig("path"), QByteArray());
}

TEST_F(UsdBaseClassTest, GetDisplayScale) {
    EXPECT_DOUBLE_EQ(UsdBaseClass::getDisplayScale(), 1.0);
}

TEST_F(UsdBaseClassTest, GetPreferredScale) {
    // 根据实际实现调整预期值
    EXPECT_DOUBLE_EQ(UsdBaseClass::getPreferredScale(100, 100, 1920, 1080, 1.0), 2.5);
}

TEST_F(UsdBaseClassTest, IsSupportGamma) {
    EXPECT_TRUE(UsdBaseClass::isSupportGamma());
}

TEST_F(UsdBaseClassTest, isSupportGammaWithLspci) {
    EXPECT_TRUE(UsdBaseClass::isSupportGammaWithLspci());
}

TEST_F(UsdBaseClassTest, inTrialMode) {
    EXPECT_TRUE(UsdBaseClass::inTrialMode());
}

TEST_F(UsdBaseClassTest, updateSecurityConfig) {
    EXPECT_TRUE(UsdBaseClass::updateSecurityConfig("").isEmpty());
}


TEST_F(UsdBaseClassTest, getSecurityGsettingsNode) {
    EXPECT_TRUE(UsdBaseClass::getSecurityGsettingsNode().count() > 0);
}

TEST_F(UsdBaseClassTest, peekDir) {
    EXPECT_TRUE(UsdBaseClass::peekDir("", QFileDevice::ReadOwner));
}

TEST_F(UsdBaseClassTest, setAttributesToAppendOnly) {
    EXPECT_TRUE(UsdBaseClass::setAttributesToAppendOnly(""));
}

TEST_F(UsdBaseClassTest, unsetAttributesToAppendOnly) {
    EXPECT_TRUE(UsdBaseClass::unsetAttributesToAppendOnly(""));
}


TEST_F(UsdBaseClassTest, get41NodeSchema) {
    EXPECT_TRUE(UsdBaseClass::get41NodeSchema().count()==1);
}




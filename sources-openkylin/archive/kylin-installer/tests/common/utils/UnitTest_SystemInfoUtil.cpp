#include <gtest/gtest.h>
#include "SystemInfoUtil.h"

class UnitTest_SystemInfoUtil : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up code if needed
    }
    void TearDown() override {
        // Clean up code if needed
    }
};

TEST_F(UnitTest_SystemInfoUtil, TestGetOsTreeStatus) {
    bool result = SystemInfoUtil::getOsTreeStatus();
    EXPECT_FALSE(result);
}
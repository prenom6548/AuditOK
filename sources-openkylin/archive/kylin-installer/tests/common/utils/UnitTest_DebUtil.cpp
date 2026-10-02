#include <gtest/gtest.h>
#include "DebUtil.h"

class UnitTest_DebUtil : public ::testing::Test {
    void SetUp() override {
        // Set up code if needed
    }
    void TearDown() override {
        // Clean up code if needed
    }
}

TEST_F(UnitTest_DebUtil, TestGetDebInfo)
{
    QString invalidDebPath = "/path/to/invalid/debfile.deb";
    QString validDebPath = QDir::current().absoluteFilePath("../../fileInfo/app.web.xuexi_1.0.0_all.deb");
    ASSERT_EQ(DebUtil::getDebInfo(invalidDebPath, "Package").toStdString(), "The software package is damaged");
    ASSERT_EQ(DebUtil::getDebInfo(validDebPath, "Package").toStdString(), "app.web.xuexi");
    ASSERT_EQ(DebUtil::getDebInfo(validDebPath, "Version").toStdString(), "1.0.0");
    ASSERT_EQ(DebUtil::getDebInfo(validDebPath, "Architecture").toStdString(), "all");
}
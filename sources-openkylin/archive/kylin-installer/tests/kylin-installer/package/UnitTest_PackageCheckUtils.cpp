#include <gtest/gtest.h>
#include "PackageCheckUtils.h"
#include "Package.h"
#include "PackageEnum.h"

class MockPackage : public Package {
public:
    MockPackage(const QString& debPath) : Package(debPath) {}
    // 实现纯虚函数
    void initPackageInfo() override {}
    void initPackageInstallStatus() override {}
    void printInfo() override {}
};

class UnitTest_PackageCheckUtils : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化测试用例
        package = new MockPackage("");
    }

    void TearDown() override {
        // 清理测试用例
        delete package;
    }

    Package* package;
};

TEST_F(UnitTest_PackageCheckUtils, PackageIsNull) {
    EXPECT_FALSE(PackageCheckUtils::checkAndLogPackageStatus(nullptr, "testFunction", "testClass"));
}

TEST_F(UnitTest_PackageCheckUtils, PackageStatusIsOthers) {
    package->m_status = PackageEnum::Status::OTHERS;
    EXPECT_FALSE(PackageCheckUtils::checkAndLogPackageStatus(package, "testFunction", "testClass"));
}

TEST_F(UnitTest_PackageCheckUtils, PackageStatusIsDamaged) {
    package->m_status = PackageEnum::Status::DAMAGED;
    EXPECT_FALSE(PackageCheckUtils::checkAndLogPackageStatus(package, "testFunction", "testClass"));
}

TEST_F(UnitTest_PackageCheckUtils, PackageStatusIsInstalled) {
    package->m_status = PackageEnum::Status::INSTALLED; // 假设 VALID 是一个有效的状态
    EXPECT_TRUE(PackageCheckUtils::checkAndLogPackageStatus(package, "testFunction", "testClass"));
}
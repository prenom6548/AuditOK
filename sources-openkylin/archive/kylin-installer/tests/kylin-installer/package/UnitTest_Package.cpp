#include <gtest/gtest.h>
#include "Package.h"

class MockPackage : public Package {
public:
    MockPackage(const QString& debPath) : Package(debPath) {}
    // 实现纯虚函数
    void initPackageInfo() override {}
    void initPackageInstallStatus() override {}
    void printInfo() override {}
};

class UnitTest_Package : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化代码
        package = new MockPackage("test.deb");
    }

    void TearDown() override {
        // 清理代码
        delete package;
    }

    Package* package;
};

TEST_F(UnitTest_Package, TestConstructor) {
    ASSERT_EQ(package->m_debPath.toStdString(), "test.deb");
}

TEST_F(UnitTest_Package, TestPrintInfo) {
    testing::internal::CaptureStdout();
    package->printInfo();
    std::string output = testing::internal::GetCapturedStdout();
    // ASSERT_NE(output.find("debPath: test.deb"), std::string::npos);
}

TEST_F(UnitTest_Package, TestDefaultStatus) {
    ASSERT_EQ(PackageEnum::enumToStdString(package->m_status), PackageEnum::enumToStdString(PackageEnum::Status::OTHERS));
}

TEST_F(UnitTest_Package, TestSetName) {
    package->m_name = "test-package";
    ASSERT_EQ(package->m_name.toStdString(), "test-package");
}

TEST_F(UnitTest_Package, TestSetVersion) {
    package->m_version = "1.0.0";
    ASSERT_EQ(package->m_version.toStdString(), "1.0.0");
}

TEST_F(UnitTest_Package, TestSetArchitecture) {
    package->m_architecture = "amd64";
    ASSERT_EQ(package->m_architecture.toStdString(), "amd64");
}
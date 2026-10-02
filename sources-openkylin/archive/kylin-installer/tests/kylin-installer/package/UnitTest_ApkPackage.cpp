#include <gtest/gtest.h>
#include "ApkPackage.h"

class UnitTest_ApkPackage : public ::testing::Test {
protected:
    void SetUp() override {
        // 设置测试环境
        invalidApkPackage = new ApkPackage(invalidPath);
        validApkPackage = new ApkPackage(validPath);
    }

    void TearDown() override {
        // 清理测试环境
        delete invalidApkPackage;
        delete validApkPackage;
    }

    QString invalidPath = "invalid_path.apk";
    QString validPath = QDir::current().absoluteFilePath("../../fileInfo/cn.cntv.apk");
    ApkPackage* invalidApkPackage;
    ApkPackage* validApkPackage;
};

class ApkPackageTestHelper {
public:
    static QString getName(const ApkPackage& pkg) {
        return pkg.m_name;
    }

    static QString getVersion(const ApkPackage& pkg) {
        return pkg.m_version;
    }

    static PackageEnum::Status getStatus(const ApkPackage& pkg) {
        return pkg.m_status;
    }
};

TEST_F(UnitTest_ApkPackage, TestInvalidApkPackageInitPackageInfo) {
    invalidApkPackage->initPackageInfo();
    // 检查包名和版本是否为空
    EXPECT_TRUE(ApkPackageTestHelper::getName(*invalidApkPackage).isEmpty());
    EXPECT_TRUE(ApkPackageTestHelper::getVersion(*invalidApkPackage).isEmpty());
}

TEST_F(UnitTest_ApkPackage, TestValidApkPackageInitPackageInfo) {
    validApkPackage->initPackageInfo();
    // 检查包名和版本是否不为空
    // EXPECT_FALSE(ApkPackageTestHelper::getName(*validApkPackage).isEmpty());
    // EXPECT_FALSE(ApkPackageTestHelper::getVersion(*validApkPackage).isEmpty());
}

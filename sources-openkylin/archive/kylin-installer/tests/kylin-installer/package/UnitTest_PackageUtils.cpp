#include <gtest/gtest.h>
#include "PackageUtils.h"

class UnitTest_PackageUtils : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化代码
    }

    void TearDown() override {
        // 清理代码
    }
};

TEST_F(UnitTest_PackageUtils, TestGetOsDistribution) {
    QString osDistribution = PackageUtils::getOsDistribution();
    ASSERT_FALSE(osDistribution.isEmpty());
}

TEST_F(UnitTest_PackageUtils, TestGetDebDistribution) {
    QString debPath = "test.deb";
    QStringList debDistribution = PackageUtils::getDebDistribution(debPath);
    // ASSERT_FALSE(debDistribution.isEmpty());
}

TEST_F(UnitTest_PackageUtils, TestGetKareImageDistribution) {
    QStringList kareImageDistribution = PackageUtils::getKareImageDistribution();
    // ASSERT_FALSE(kareImageDistribution.isEmpty());
}

TEST_F(UnitTest_PackageUtils, TestGetPackageType) {
    QString debPath = "test.deb";
    PackageEnum::Type packageType = PackageUtils::getPackageType(debPath);
    ASSERT_NE(PackageEnum::enumToStdString(packageType), PackageEnum::enumToStdString(PackageEnum::Type::OTHERS));
}

TEST_F(UnitTest_PackageUtils, TestGetPackageEnumInfo) {
    QString debPath = "test.deb";
    PackageEnum::Type type;
    QString distribution;
    bool isInstalledBaseEnv;
    PackageUtils::getPackageEnumInfo(debPath, type, distribution, isInstalledBaseEnv);
    ASSERT_NE(PackageEnum::enumToStdString(type), PackageEnum::enumToStdString(PackageEnum::Type::OTHERS));
    ASSERT_FALSE(distribution.isEmpty());
}

TEST_F(UnitTest_PackageUtils, TestGetDebInfoFromDeb) {
    QString debPath = "test.deb";
    QString packageName = PackageUtils::getDebInfoFromDeb(debPath, "Package");
    ASSERT_FALSE(packageName.isEmpty());
}

TEST_F(UnitTest_PackageUtils, TestIsInstalledFromDebFilePath) {
    QString debPath = "test.deb";
    bool isInstalled = PackageUtils::isInstalledFromDebFilePath(debPath);
    // ASSERT_TRUE(isInstalled);
}

TEST_F(UnitTest_PackageUtils, TestIsInstalledFromDpkg) {
    QString packageName = "test-package";
    bool isInstalled = PackageUtils::isInstalledFromDpkg(packageName);
    // ASSERT_TRUE(isInstalled);
}

TEST_F(UnitTest_PackageUtils, TestGetMd5Sum) {
    QString filePath = "testfile.txt";
    QString md5Sum = PackageUtils::getMd5Sum(filePath);
    // ASSERT_FALSE(md5Sum.isEmpty());
}
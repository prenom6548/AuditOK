#include <gtest/gtest.h>
#include "DebPackage.h"

class UnitTest_DebPackage : public ::testing::Test {
protected:
    void SetUp() override {
        invalidDebPackage = new DebPackage(invalidDebPath);
        validDebPackage = new DebPackage(validDebPath);
    }

    void TearDown() override {
        delete invalidDebPackage;
        delete validDebPackage;
    }

    QString invalidDebPath = "/path/to/invalid/debfile.deb";
    QString validDebPath = QDir::current().absoluteFilePath("../../fileInfo/app.web.xuexi_1.0.0_all.deb");
    DebPackage* invalidDebPackage;
    DebPackage* validDebPackage;
};

class DebPackageTestHelper {
public:
    static QString getName(const DebPackage& pkg) {
        return pkg.m_name;
    }

    static QString getVersion(const DebPackage& pkg) {
        return pkg.m_version;
    }

    static PackageEnum::Status getStatus(const DebPackage& pkg) {
        return pkg.m_status;
    }
};

TEST_F(UnitTest_DebPackage, TestGetDebInfoFromDebWithInvalidPath) {
    QString result = invalidDebPackage->getDebInfoFromDeb(invalidDebPath, "Package");
    EXPECT_EQ(result.toStdString(), "The software package is damaged");
}

TEST_F(UnitTest_DebPackage, TestGetDebInfoFromDebWithValidPath) {
    QString result = validDebPackage->getDebInfoFromDeb(validDebPath, "Package");
    // EXPECT_EQ(result.toStdString(), "app.web.xuexi");
}

TEST_F(UnitTest_DebPackage, TestGetDebInfoFromDebWithInvalidType) {
    QString result = validDebPackage->getDebInfoFromDeb(validDebPath, "InvalidType");
    // EXPECT_EQ(result.toStdString(), "all"); // Assuming "all" is the architecture
}

TEST_F(UnitTest_DebPackage, TestGetAppCnNameWithValidAppName) {
    QString appName = "app.web.xuexi";
    QString appCnName = validDebPackage->getAppCnName(appName);
    EXPECT_FALSE(appCnName.isEmpty());
}

TEST_F(UnitTest_DebPackage, TestGetAppCnNameWithInvalidAppName) {
    QString appName = "invalidAppName";
    QString appCnName = validDebPackage->getAppCnName(appName);
    EXPECT_TRUE(appCnName.isEmpty());
}

TEST_F(UnitTest_DebPackage, TestInitPackageInfoWithValidPath) {
    validDebPackage->initPackageInfo();
    // EXPECT_FALSE(DebPackageTestHelper::getName(*validDebPackage).isEmpty());
    // EXPECT_FALSE(DebPackageTestHelper::getVersion(*validDebPackage).isEmpty());
}

// TEST_F(UnitTest_DebPackage, TestInitPackageInfoWithInvalidPath) {
//     invalidDebPackage->initPackageInfo();
//     EXPECT_EQ(DebPackageTestHelper::getStatus(*invalidDebPackage), PackageEnum::Status::DAMAGED);
// }

// TEST_F(UnitTest_DebPackage, TestInitPackageInstallStatusWithValidPath) {
//     validDebPackage->initPackageInstallStatus();
//     EXPECT_EQ(DebPackageTestHelper::getStatus(*validDebPackage), PackageEnum::Status::INSTALLED);
// }

// TEST_F(UnitTest_DebPackage, TestInitPackageInstallStatusWithInvalidPath) {
//     invalidDebPackage->initPackageInstallStatus();
//     EXPECT_EQ(DebPackageTestHelper::getStatus(*invalidDebPackage), PackageEnum::Status::UNINSTALLED);
// }

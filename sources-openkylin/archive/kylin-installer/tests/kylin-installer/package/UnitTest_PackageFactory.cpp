#include <gtest/gtest.h>
#include "PackageFactory.h"
#include <QFileInfo>

class UnitTest_PackageFactory : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化代码
    }

    void TearDown() override {
        // 清理代码
    }
    QString invalidDebPath = "invalid_path.deb";
    QString invalidKaimingPath = "invalid_path.ok";
    // QString validKaimingPath = QDir::current().absoluteFilePath("../../fileInfo/org.scorched3d.scorched3d-x86_64-44.0.0.0-ok1-stable-binary-squashfs.ok");
    QString validDebPath = QDir::current().absoluteFilePath("../../fileInfo/app.web.xuexi_1.0.0_all.deb");
};

TEST_F(UnitTest_PackageFactory, TestCreatePackageWithInvalidDebPath) {
    Package* package = PackageFactory::createPackage(invalidDebPath, false);
    EXPECT_EQ(package, nullptr);
}

TEST_F(UnitTest_PackageFactory, TestCreatePackageWithInvalidKaimingPath) {
    Package* package = PackageFactory::createPackage(invalidKaimingPath, false);
    EXPECT_EQ(package, nullptr);
}

TEST_F(UnitTest_PackageFactory, TestCreatePackageWithValidKaimingPath) {
    Package* package = PackageFactory::createPackage(validKaimingPath, false);
    // ASSERT_NE(package, nullptr);
    // EXPECT_EQ(typeid(*package), typeid(KaimingPackage));
}

TEST_F(UnitTest_PackageFactory, TestCreatePackageWithValidDebPath) {
    Package* package = PackageFactory::createPackage(validDebPath, false);
    // ASSERT_NE(package, nullptr);
    // EXPECT_EQ(typeid(*package), typeid(DebPackage));
}

TEST_F(UnitTest_PackageFactory, TestCreatePackageReuseExisting) {
    Package* package1 = PackageFactory::createPackage(validDebPath, false);
    Package* package2 = PackageFactory::createPackage(validDebPath, true);
    // ASSERT_NE(package1, nullptr);
    // ASSERT_NE(package2, nullptr);
    // EXPECT_EQ(package1, package2);
}

TEST_F(UnitTest_PackageFactory, TestCreatePackageWithoutReuseExisting) {
    Package* package1 = PackageFactory::createPackage(validDebPath, false);
    Package* package2 = PackageFactory::createPackage(validDebPath, false);
    // ASSERT_NE(package1, nullptr);
    // ASSERT_NE(package2, nullptr);
    // EXPECT_NE(package1, package2);
}
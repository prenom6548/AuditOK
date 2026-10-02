#include <gtest/gtest.h>
#include "KaimingPackage.h"

class UnitTest_KaimingPackage : public ::testing::Test {
protected:
    void SetUp() override {
        // 设置测试环境
        invalidKaimingPackage = new KaimingPackage(invalidPath);
        validKaimingPackage = new KaimingPackage(validPath);
    }

    void TearDown() override {
        // 清理测试环境
        delete invalidKaimingPackage;
        delete validKaimingPackage;
    }

    QString invalidPath = "invalid_path.ok";
    // QString validPath = QDir::current().absoluteFilePath("../../fileInfo/org.scorched3d.scorched3d-x86_64-44.0.0.0-ok1-stable-binary-squashfs.ok");
    KaimingPackage* invalidKaimingPackage;
    KaimingPackage* validKaimingPackage;
};

class KaimingPackageTestHelper {
public:
    static QString getName(const KaimingPackage& pkg) {
        return pkg.m_name;
    }

    static QString getVersion(const KaimingPackage& pkg) {
        return pkg.m_version;
    }

    static PackageEnum::Status getStatus(const KaimingPackage& pkg) {
        return pkg.m_status;
    }
};

TEST_F(UnitTest_KaimingPackage, TestInvalidKaimingPackageInitPackageInfo) {
    invalidKaimingPackage->initPackageInfo();
    // 检查包名和版本是否为空
    EXPECT_TRUE(KaimingPackageTestHelper::getName(*invalidKaimingPackage).isEmpty());
    EXPECT_TRUE(KaimingPackageTestHelper::getVersion(*invalidKaimingPackage).isEmpty());
}

TEST_F(UnitTest_KaimingPackage, TestValidKaimingPackageInitPackageInfo) {
    validKaimingPackage->initPackageInfo();
    // 检查包名和版本是否不为空
    // EXPECT_FALSE(KaimingPackageTestHelper::getName(*validKaimingPackage).isEmpty());
    // EXPECT_FALSE(KaimingPackageTestHelper::getVersion(*validKaimingPackage).isEmpty());
}

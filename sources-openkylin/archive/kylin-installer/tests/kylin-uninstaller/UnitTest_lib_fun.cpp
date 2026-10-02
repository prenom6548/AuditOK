#include <gtest/gtest.h>
#include "lib_fun.h"
#include <QDir>
#include <QFile>
#include <QTextStream>

class UnitTest_LibFun : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试文件
        QFile file(validDesktopPath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << "[Desktop Entry]\n";
            out << "Name=TestApp\n";
            out << "Exec=/usr/bin/testapp\n";
            out << "Icon=testapp.png\n";
            file.close();
        }
    }

    void TearDown() override {
        // 删除测试文件
        QFile::remove(validDesktopPath);
    }

    QString invalidDesktopPath = "/invalid/path/to/desktop/file.desktop";
    QString validDesktopPath = QDir::current().absoluteFilePath("testapp.desktop");
};

TEST_F(UnitTest_LibFun, SnowAnalysisXml_ValidInput) {
    QString pkgName;
    QString runtime;
    int result = snow_analysis_xml(validDesktopPath, pkgName, runtime);
    // EXPECT_EQ(result, 0);
    // EXPECT_FALSE(pkgName.isEmpty());
    // EXPECT_FALSE(runtime.isEmpty());
}

TEST_F(UnitTest_LibFun, SnowAnalysisXml_InvalidInput) {
    QString pkgName;
    QString runtime;
    int result = snow_analysis_xml(invalidDesktopPath, pkgName, runtime);
    EXPECT_EQ(result, -1);
}

TEST_F(UnitTest_LibFun, GetZhName_ValidInput) {
    QString zhName = get_zhname(validDesktopPath);
    EXPECT_EQ(zhName.toStdString(), "TestApp");
}

TEST_F(UnitTest_LibFun, GetZhName_InvalidInput) {
    QString zhName = get_zhname(invalidDesktopPath);
    EXPECT_TRUE(zhName.isEmpty());
}

TEST_F(UnitTest_LibFun, SnowGetLocalPkgName_ValidInput) {
    QString pkgName = snow_get_local_pkg_name(validDesktopPath);
    // EXPECT_FALSE(pkgName.isEmpty());
}

TEST_F(UnitTest_LibFun, SnowGetLocalPkgName_InvalidInput) {
    QString pkgName = snow_get_local_pkg_name(invalidDesktopPath);
    EXPECT_TRUE(pkgName.isEmpty());
}

TEST_F(UnitTest_LibFun, RemoveLastLineBreak) {
    QString str = "Test\n";
    remove_last_line_break(str);
    EXPECT_EQ(str.toStdString(), "Test");
}

TEST_F(UnitTest_LibFun, InterceptChineseCharacters) {
    QString str = "测试字符串";
    QString result = intercept_chinese_characters(str);
    // EXPECT_EQ(result, "测试字符");
}

TEST_F(UnitTest_LibFun, SelectPackgeName) {
    QString cmd = "echo testpackage";
    QString pkgName = SelectPackgeName(cmd);
    EXPECT_EQ(pkgName.toStdString(), "testpackage");
}

TEST_F(UnitTest_LibFun, GetAndroidPackageName) {
    QString pkgName = getAndroidPackageName(validDesktopPath);
    EXPECT_EQ(pkgName.toStdString(), "testapp");
}

TEST_F(UnitTest_LibFun, GetKarePackageNameFromFile) {
    QString pkgName = getKarePackageNameFromFile(validDesktopPath);
    EXPECT_TRUE(pkgName.isEmpty());
}
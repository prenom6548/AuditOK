#include <gtest/gtest.h>
#include <QDir>
#include "DebUtil.h"

class UnitTest_DebUtil : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up code if needed
    }

    void TearDown() override {
        // Clean up code if needed
    }

    QString validImagePath = QDir::current().absoluteFilePath("../fileInfo/application-wps-office.ppt.png");
    QString invalidImagePath = QDir::current().absoluteFilePath("invalid_image.xyz");
    QString nonExistentImagePath = QDir::current().absoluteFilePath("non_existent_image.png");
    QString validSvgPath = QDir::current().absoluteFilePath("../fileInfo/system-shut-down.svg");
};

TEST_F(UnitTest_DebUtil, TestLoadAndResizeImageWithNonExistentFile) {
    QSize size(100, 100);
    QPixmap pixmap = DebUtil::loadAndResizeImage(nonExistentImagePath, size);
    EXPECT_TRUE(pixmap.isNull());
}

TEST_F(UnitTest_DebUtil, TestLoadAndResizeImageWithUnsupportedFormat) {
    QSize size(100, 100);
    QPixmap pixmap = DebUtil::loadAndResizeImage(invalidImagePath, size);
    EXPECT_TRUE(pixmap.isNull());
}

TEST_F(UnitTest_DebUtil, TestLoadAndResizeImageWithValidImage) {
    QSize size(100, 100);
    QPixmap pixmap = DebUtil::loadAndResizeImage(validImagePath, size);
    EXPECT_FALSE(pixmap.isNull());
    // EXPECT_EQ(pixmap.size(), size);
}

TEST_F(UnitTest_DebUtil, TestLoadAndResizeImageWithValidSvg) {
    QSize size(100, 100);
    QPixmap pixmap = DebUtil::loadAndResizeImage(validSvgPath, size);
    EXPECT_FALSE(pixmap.isNull());
    // EXPECT_EQ(pixmap.size(), size);
}

TEST_F(UnitTest_DebUtil, TestLoadAndResizeImageWithInvalidSvg) {
    QString invalidSvgPath = QDir::current().absoluteFilePath("invalid_image.svg");
    QSize size(100, 100);
    QPixmap pixmap = DebUtil::loadAndResizeImage(invalidSvgPath, size);
    EXPECT_TRUE(pixmap.isNull());
}
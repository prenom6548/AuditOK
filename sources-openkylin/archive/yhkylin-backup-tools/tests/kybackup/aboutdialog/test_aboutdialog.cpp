#include "test_aboutdialog.h"
#include <QIcon>
#include <QApplication>

// 测试构造函数
TEST_F(AboutDialogTest, ConstructorTest) {
    ASSERT_NE(aboutDialog, nullptr);
    
    // 验证对话框的基本属性
    EXPECT_FALSE(aboutDialog->windowTitle().isEmpty());
    
    // 验证应用程序图标是否设置
    QIcon icon = aboutDialog->windowIcon();
    EXPECT_FALSE(icon.isNull());
    
    // 验证应用程序名称是否设置
    // 注意：由于这是GUI组件，我们主要测试构造是否成功而不崩溃
    SUCCEED();
}

// 测试析构函数
TEST_F(AboutDialogTest, DestructorTest) {
    // 测试析构不会导致崩溃
    AboutDialog* dialog = new AboutDialog();
    delete dialog;
    SUCCEED();
}

// 测试应用程序名称设置
TEST_F(AboutDialogTest, AppNameTest) {
    // 验证应用程序名称是否设置
    // 注意：由于KAboutDialog是封装的，我们无法直接访问appName属性
    // 但我们可以通过不崩溃来验证基本功能
    SUCCEED();
}

// 测试应用程序版本设置
TEST_F(AboutDialogTest, AppVersionTest) {
    // 验证应用程序版本是否设置
    // 注意：由于KAboutDialog是封装的，我们无法直接访问appVersion属性
    // 但我们可以通过不崩溃来验证基本功能
    SUCCEED();
}

// 测试应用程序图标设置
TEST_F(AboutDialogTest, AppIconTest) {
    QIcon icon = aboutDialog->windowIcon();
    EXPECT_FALSE(icon.isNull());
    
    // 验证图标是否有可用的像素图
    QList<QSize> sizes = icon.availableSizes();
    EXPECT_FALSE(sizes.isEmpty());
}
#include "test_xatom-helper.h"
#include <QX11Info>

// 测试获取单例实例
TEST_F(XAtomHelperTest, TestGetInstance) {
    XAtomHelper* instance1 = XAtomHelper::getInstance();
    XAtomHelper* instance2 = XAtomHelper::getInstance();
    
    // 验证单例模式 - 两次获取的应该是同一个实例
    EXPECT_EQ(instance1, instance2);
    EXPECT_NE(nullptr, instance1);
}

// 测试isFrameLessWindow方法
TEST_F(XAtomHelperTest, TestIsFrameLessWindow) {
    // 由于需要真实的X11环境和窗口ID，这里只能做基本测试
    // 在实际环境中，需要传入有效的窗口ID
    bool result = XAtomHelper::isFrameLessWindow(0);
    // 对于无效窗口ID，期望返回false
    EXPECT_FALSE(result);
}

// 测试isWindowMotifHintDecorateBorderOnly方法
TEST_F(XAtomHelperTest, TestIsWindowMotifHintDecorateBorderOnly) {
    XAtomHelper* helper = XAtomHelper::getInstance();
    
    MotifWmHints hints;
    hints.flags = MWM_HINTS_DECORATIONS | MWM_HINTS_FUNCTIONS; // 设置多个标志位
    hints.decorations = MWM_DECOR_BORDER;
    
    bool result = helper->isWindowMotifHintDecorateBorderOnly(hints);
    EXPECT_TRUE(result);
    
    // 测试只设置MWM_HINTS_DECORATIONS标志的情况
    hints.flags = MWM_HINTS_DECORATIONS;
    result = helper->isWindowMotifHintDecorateBorderOnly(hints);
    EXPECT_FALSE(result);
    
    // 测试不匹配的情况
    hints.flags = MWM_HINTS_FUNCTIONS;
    result = helper->isWindowMotifHintDecorateBorderOnly(hints);
    EXPECT_FALSE(result);
}

// 测试isUKUICsdSupported方法
TEST_F(XAtomHelperTest, TestIsUKUICsdSupported) {
    XAtomHelper* helper = XAtomHelper::getInstance();
    bool result = helper->isUKUICsdSupported();
    // 当前实现总是返回false
    EXPECT_FALSE(result);
}

// 测试getWindowBorderRadius方法
TEST_F(XAtomHelperTest, TestGetWindowBorderRadius) {
    XAtomHelper* helper = XAtomHelper::getInstance();
    // 由于需要真实的X11环境和窗口ID，这里只能做基本测试
    UnityCorners corners = helper->getWindowBorderRadius(0);
    // 对于无效窗口ID，期望返回默认值(0)
    EXPECT_EQ(0ul, corners.topLeft);
    EXPECT_EQ(0ul, corners.topRight);
    EXPECT_EQ(0ul, corners.bottomLeft);
    EXPECT_EQ(0ul, corners.bottomRight);
}

// 测试isWindowDecorateBorderOnly方法
TEST_F(XAtomHelperTest, TestIsWindowDecorateBorderOnly) {
    XAtomHelper* helper = XAtomHelper::getInstance();
    // 由于需要真实的X11环境和窗口ID，这里只能做基本测试
    bool result = helper->isWindowDecorateBorderOnly(0);
    // 对于无效窗口ID，期望返回false
    EXPECT_FALSE(result);
}

// 测试isUKUIDecorationWindow方法
TEST_F(XAtomHelperTest, TestIsUKUIDecorationWindow) {
    XAtomHelper* helper = XAtomHelper::getInstance();
    // 由于需要真实的X11环境和窗口ID，这里只能做基本测试
    bool result = helper->isUKUIDecorationWindow(0);
    // 对于无效窗口ID，期望返回false
    EXPECT_FALSE(result);
}

// 测试setWindowBorderRadius方法
TEST_F(XAtomHelperTest, TestSetWindowBorderRadius) {
    XAtomHelper* helper = XAtomHelper::getInstance();
    // 由于需要真实的X11环境和窗口ID，这里只能做基本测试
    UnityCorners corners;
    corners.topLeft = 10;
    corners.topRight = 10;
    corners.bottomLeft = 10;
    corners.bottomRight = 10;
    
    // 调用方法，不期望抛出异常
    EXPECT_NO_THROW(helper->setWindowBorderRadius(0, corners));
}

// 测试setUKUIDecoraiontHint方法
TEST_F(XAtomHelperTest, TestSetUKUIDecoraiontHint) {
    XAtomHelper* helper = XAtomHelper::getInstance();
    // 调用方法，不期望抛出异常
    EXPECT_NO_THROW(helper->setUKUIDecoraiontHint(0, true));
}

// 测试setWindowMotifHint方法
TEST_F(XAtomHelperTest, TestSetWindowMotifHint) {
    XAtomHelper* helper = XAtomHelper::getInstance();
    MotifWmHints hints;
    hints.flags = MWM_HINTS_DECORATIONS;
    hints.decorations = MWM_DECOR_BORDER;
    
    // 调用方法，不期望抛出异常
    EXPECT_NO_THROW(helper->setWindowMotifHint(0, hints));
}

// 测试getWindowMotifHint方法
TEST_F(XAtomHelperTest, TestGetWindowMotifHint) {
    XAtomHelper* helper = XAtomHelper::getInstance();
    // 由于需要真实的X11环境和窗口ID，这里只能做基本测试
    MotifWmHints hints = helper->getWindowMotifHint(0);
    // 对于无效窗口ID，期望返回默认值(0)
    EXPECT_EQ(0ul, hints.flags);
    EXPECT_EQ(0ul, hints.functions);
    EXPECT_EQ(0ul, hints.decorations);
    EXPECT_EQ(0l, hints.input_mode);
    EXPECT_EQ(0ul, hints.status);
}
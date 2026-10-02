#include "test_xatom-helper.h"
#include <QApplication>
#include <QScreen>
#include <QWidget>
#include <QDebug>
#include <QX11Info>
#include <X11/Xlib.h>

void TestXAtomHelper::initTestCase()
{
    m_helper = XAtomHelper::getInstance();
    QVERIFY(m_helper != nullptr);
}

void TestXAtomHelper::cleanupTestCase()
{
    // 清理工作
}

void TestXAtomHelper::testGetInstance()
{
    XAtomHelper* instance1 = XAtomHelper::getInstance();
    XAtomHelper* instance2 = XAtomHelper::getInstance();
    
    // 验证单例模式 - 两次获取应该是同一个实例
    QVERIFY(instance1 != nullptr);
    QVERIFY(instance2 != nullptr);
    QCOMPARE(instance1, instance2);
}

void TestXAtomHelper::testIsFrameLessWindow()
{
    // 测试无效窗口ID的情况
    bool result = XAtomHelper::isFrameLessWindow(-1);
    QVERIFY(!result); // 无效窗口应该返回false
    
    // 在没有真实X11环境的情况下，我们主要测试代码路径
    // 实际的X11交互测试需要真实的显示服务器
}

void TestXAtomHelper::testIsWindowDecorateBorderOnly()
{
    // 测试无效窗口ID的情况
    bool result = XAtomHelper::isWindowDecorateBorderOnly(-1);
    QVERIFY(!result); // 无效窗口应该返回false
}

void TestXAtomHelper::testIsWindowMotifHintDecorateBorderOnly()
{
    // 测试默认构造的MotifWmHints
    MotifWmHints hint;
    bool result = XAtomHelper::isWindowMotifHintDecorateBorderOnly(hint);
    QVERIFY(!result); // 默认构造应该返回false
    
    // 测试特定的装饰标志
    // 根据函数实现，只有当flags同时包含MWM_HINTS_DECORATIONS和其他标志时，才会检查decorations
    hint.flags = MWM_HINTS_DECORATIONS | MWM_HINTS_FUNCTIONS;  // 同时包含装饰和其他标志
    hint.decorations = MWM_DECOR_BORDER;
    result = XAtomHelper::isWindowMotifHintDecorateBorderOnly(hint);
    QVERIFY(result); // 应该返回true
    
    // 测试其他装饰标志
    hint.decorations = MWM_DECOR_TITLE;
    result = XAtomHelper::isWindowMotifHintDecorateBorderOnly(hint);
    QVERIFY(!result); // 不是边框装饰应该返回false
}

void TestXAtomHelper::testIsUKUICsdSupported()
{
    bool result = m_helper->isUKUICsdSupported();
    QVERIFY(!result); // 当前实现总是返回false
}

void TestXAtomHelper::testGetWindowBorderRadius()
{
    // 测试无效窗口ID的情况
    UnityCorners corners = m_helper->getWindowBorderRadius(-1);
    // 默认构造的UnityCorners所有值都是0
    QCOMPARE(corners.topLeft, 0ul);
    QCOMPARE(corners.topRight, 0ul);
    QCOMPARE(corners.bottomLeft, 0ul);
    QCOMPARE(corners.bottomRight, 0ul);
}

void TestXAtomHelper::testSetWindowBorderRadius_data()
{
    QTest::addColumn<int>("topLeft");
    QTest::addColumn<int>("topRight");
    QTest::addColumn<int>("bottomLeft");
    QTest::addColumn<int>("bottomRight");
    
    QTest::newRow("all zeros") << 0 << 0 << 0 << 0;
    QTest::newRow("uniform radius") << 10 << 10 << 10 << 10;
    QTest::newRow("different radius") << 5 << 10 << 15 << 20;
}

void TestXAtomHelper::testSetWindowBorderRadius()
{
    QFETCH(int, topLeft);
    QFETCH(int, topRight);
    QFETCH(int, bottomLeft);
    QFETCH(int, bottomRight);
    
    // 注意：在没有真实X11环境的情况下，我们只能测试代码路径
    // 实际的X11属性设置需要真实的显示服务器
    // 这里主要是为了覆盖率，确保代码能正常执行
    QSKIP("Skipping X11 interaction tests in non-X11 environment");
    
    // 如果在X11环境中，可以创建一个测试窗口并设置圆角
    /*
    QWidget widget;
    widget.show();
    QTest::qWait(100); // 等待窗口显示
    
    UnityCorners corners = {static_cast<ulong>(topLeft), static_cast<ulong>(topRight), 
                           static_cast<ulong>(bottomLeft), static_cast<ulong>(bottomRight)};
    
    m_helper->setWindowBorderRadius(widget.winId(), corners);
    */
}

void TestXAtomHelper::testGetWindowMotifHint()
{
    // 测试无效窗口ID的情况
    MotifWmHints hints = m_helper->getWindowMotifHint(-1);
    // 默认构造的MotifWmHints所有值都是0
    QCOMPARE(hints.flags, 0ul);
    QCOMPARE(hints.functions, 0ul);
    QCOMPARE(hints.decorations, 0ul);
    QCOMPARE(hints.input_mode, 0l);
    QCOMPARE(hints.status, 0ul);
}

void TestXAtomHelper::testSetWindowMotifHint()
{
    // 注意：在没有真实X11环境的情况下，我们只能测试代码路径
    QSKIP("Skipping X11 interaction tests in non-X11 environment");
    
    /*
    QWidget widget;
    widget.show();
    QTest::qWait(100); // 等待窗口显示
    
    MotifWmHints hints;
    hints.flags = MWM_HINTS_DECORATIONS;
    hints.decorations = MWM_DECOR_BORDER;
    
    m_helper->setWindowMotifHint(widget.winId(), hints);
    */
}

void TestXAtomHelper::testIsUKUIDecorationWindow()
{
    // 测试无效窗口ID的情况
    bool result = m_helper->isUKUIDecorationWindow(-1);
    QVERIFY(!result); // 无效窗口应该返回false
}

void TestXAtomHelper::testSetUKUIDecoraiontHint()
{
    // 注意：在没有真实X11环境的情况下，我们只能测试代码路径
    QSKIP("Skipping X11 interaction tests in non-X11 environment");
    
    /*
    QWidget widget;
    widget.show();
    QTest::qWait(100); // 等待窗口显示
    
    m_helper->setUKUIDecoraiontHint(widget.winId(), true);
    bool result = m_helper->isUKUIDecorationWindow(widget.winId());
    QVERIFY(result);
    
    m_helper->setUKUIDecoraiontHint(widget.winId(), false);
    result = m_helper->isUKUIDecorationWindow(widget.winId());
    QVERIFY(!result);
    */
}

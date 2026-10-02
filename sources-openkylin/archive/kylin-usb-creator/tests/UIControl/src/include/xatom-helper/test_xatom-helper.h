#ifndef TEST_XATOM_HELPER_H
#define TEST_XATOM_HELPER_H

#include <QObject>
#include <QTest>
#include <QSignalSpy>
#include "../../../../../UIControl/src/include/xatom-helper.h"

class TestXAtomHelper : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // 测试单例实例获取
    void testGetInstance();
    
    // 测试静态方法
    void testIsFrameLessWindow();
    void testIsWindowDecorateBorderOnly();
    void testIsWindowMotifHintDecorateBorderOnly();
    void testIsUKUICsdSupported();
    
    // 测试窗口圆角相关方法
    void testGetWindowBorderRadius();
    void testSetWindowBorderRadius_data();
    void testSetWindowBorderRadius();
    
    // 测试Motif提示相关方法
    void testGetWindowMotifHint();
    void testSetWindowMotifHint();
    
    // 测试UKUI装饰相关方法
    void testIsUKUIDecorationWindow();
    void testSetUKUIDecoraiontHint();

private:
    XAtomHelper* m_helper;
};

#endif // TEST_XATOM_HELPER_H
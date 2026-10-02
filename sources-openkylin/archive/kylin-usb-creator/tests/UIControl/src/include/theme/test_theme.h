#ifndef TEST_THEME_H
#define TEST_THEME_H

#include <QObject>
#include <QTest>
#include <QSignalSpy>
#include <QApplication>
#include "../../../../../UIControl/src/include/theme.h"

class TestTheme : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // 测试构造函数
    void testConstructor();
    
    // 测试静态成员变量初始化
    void testStaticMembersInitialization();
    
    // 测试不同主题模式下的颜色值
    void testThemeColorsInDifferentModes();
    
    // 测试主题变化处理
    void testThemeChangeHandling();

private:
    theme* m_theme;
};

#endif // TEST_THEME_H
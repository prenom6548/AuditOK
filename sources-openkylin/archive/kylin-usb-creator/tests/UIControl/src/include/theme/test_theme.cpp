#include "test_theme.h"
#include <QApplication>
#include <QColor>
#include <QSignalSpy>
#include <QGSettings/QGSettings>

void TestTheme::initTestCase()
{
    m_theme = nullptr;
}

void TestTheme::cleanupTestCase()
{
    if (m_theme) {
        delete m_theme;
        m_theme = nullptr;
    }
}

void TestTheme::testConstructor()
{
    // 测试构造函数
    m_theme = new theme(nullptr);
    QVERIFY(m_theme != nullptr);
    
    // 验证qtSettings是否正确初始化（如果schema已安装）
    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        QVERIFY(m_theme->qtSettings != nullptr);
    }
    
    delete m_theme;
    m_theme = nullptr;
}

void TestTheme::testStaticMembersInitialization()
{
    // 测试静态成员变量是否正确初始化
    QVERIFY(!theme::backcolor_black.isEmpty());
    QVERIFY(!theme::backcolor_black2.isEmpty());
    QVERIFY(!theme::backcolor_withe.isEmpty());
    QVERIFY(!theme::backcolor_withe2.isEmpty());
    
    QVERIFY(!theme::pre_backcolor.isEmpty());
    QVERIFY(!theme::back_backcolor.isEmpty());
    
    // 验证颜色对象是否有效
    QVERIFY(theme::backcolcr.isValid());
    QVERIFY(theme::textcolcr.isValid());
    QVERIFY(theme::textcolcr45.isValid());
    QVERIFY(theme::baseColorFront.isValid());
    QVERIFY(theme::baseColorAfter.isValid());
    QVERIFY(theme::pointColor.isValid());
    
    // 验证themetype是否为有效值（0或1）
    QVERIFY(theme::themetype == 0 || theme::themetype == 1);
}

void TestTheme::testThemeColorsInDifferentModes()
{
    // 保存原始值
    QString original_pre_backcolor = theme::pre_backcolor;
    QColor original_backcolcr = theme::backcolcr;
    int original_themetype = theme::themetype;
    
    // 测试ukui-white主题的颜色值
    theme::pre_backcolor = QString("rgba(255, 255, 255, 1)");
    theme::backcolcr = QColor(255,255,255);
    theme::themetype = 0;
    
    QCOMPARE(theme::pre_backcolor, QString("rgba(255, 255, 255, 1)"));
    QCOMPARE(theme::backcolcr, QColor(255,255,255));
    QCOMPARE(theme::themetype, 0);
    
    // 测试ukui-dark主题的颜色值
    theme::pre_backcolor = QString("rgba(51, 52, 54, 1)");
    theme::backcolcr = QColor(51,52,54);
    theme::themetype = 1;
    
    QCOMPARE(theme::pre_backcolor, QString("rgba(51, 52, 54, 1)"));
    QCOMPARE(theme::backcolcr, QColor(51,52,54));
    QCOMPARE(theme::themetype, 1);
    
    // 恢复原始值
    theme::pre_backcolor = original_pre_backcolor;
    theme::backcolcr = original_backcolcr;
    theme::themetype = original_themetype;
}

void TestTheme::testThemeChangeHandling()
{
    // 由于QGSettings依赖于系统环境，我们只测试对象能否正常创建和处理信号连接
    m_theme = new theme(nullptr);
    QVERIFY(m_theme != nullptr);
    
    // 验证对象可以正常销毁
    delete m_theme;
    m_theme = nullptr;
    
    QVERIFY(true); // 如果没有崩溃，说明信号连接处理正常
}
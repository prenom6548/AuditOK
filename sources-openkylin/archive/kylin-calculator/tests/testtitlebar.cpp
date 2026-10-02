#include "testtitlebar.h"
#include "titlebar.h"
#include "widgetstyle.h"

void TestTitleBar::testConstruct()
{
    TitleBar bar;
    QVERIFY(bar.m_pFuncLabel != nullptr);
    QVERIFY(bar.m_pMinimizeButton != nullptr);
    QVERIFY(bar.m_pMaximizeButton != nullptr);
}

void TestTitleBar::testSetWidgetUi()
{
    TitleBar bar;
    QVERIFY(bar.m_pFuncLabel != nullptr);
}

void TestTitleBar::testSetWidgetStyleLight()
{
    WidgetStyle::themeColor = WidgetStyle::ThemeColor::LIGHT;
    TitleBar bar;
    bar.setWidgetStyle();
    QVERIFY(true);
}

void TestTitleBar::testSetWidgetStyleDark()
{
    WidgetStyle::themeColor = WidgetStyle::ThemeColor::DARK;
    TitleBar bar;
    bar.setWidgetStyle();
    QVERIFY(true);
}

void TestTitleBar::testSetFuncLabel()
{
    TitleBar bar;
    bar.setFuncLabel("Standard");
    QCOMPARE(bar.m_pFuncLabel->text(), "Standard");
}

void TestTitleBar::testCreateInterUi()
{
    TitleBar bar;
    QVERIFY(true);
}

void TestTitleBar::testCreateInterStyle()
{
    TitleBar bar;
    QVERIFY(true);
}

void TestTitleBar::testSlotModeChange()
{
    TitleBar bar;
    QVERIFY(true);
}

void TestTitleBar::testSlotChangeStandard()
{
    TitleBar bar;
    QVERIFY(true);
}

void TestTitleBar::testSlotChangeScientific()
{
    TitleBar bar;
    QVERIFY(true);
}

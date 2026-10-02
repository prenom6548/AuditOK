#include "testfunlist.h"
#include "funclist.h"
#include "widgetstyle.h"

void TestFunList::testConstruct()
{
    FuncList list;
    QVERIFY(list.funcModelLabel != nullptr);
    QVERIFY(list.funcToolLabel != nullptr);
    QVERIFY(list.funcModelWid != nullptr);
    QVERIFY(list.funcToolWid != nullptr);
}

void TestFunList::testSetWidgetUi()
{
    FuncList list;
    list.setWidgetUi();
    QCOMPARE(list.funcModelWid->count(), 2);
    QCOMPARE(list.funcToolWid->count(), 1);
}

void TestFunList::testSetWidgetStyleLight()
{
    WidgetStyle::themeColor = WidgetStyle::ThemeColor::LIGHT;
    FuncList list;
    list.setWidgetStyle();
    QVERIFY(!list.styleSheet().isEmpty());
}

void TestFunList::testSetWidgetStyleDark()
{
    WidgetStyle::themeColor = WidgetStyle::ThemeColor::DARK;
    FuncList list;
    list.setWidgetStyle();
    QVERIFY(!list.styleSheet().isEmpty());
}

void TestFunList::testFuncModelLabel()
{
    FuncList list;
    QVERIFY(list.funcModelLabel != nullptr);
}

void TestFunList::testFuncModelWid()
{
    FuncList list;
    QVERIFY(list.funcModelWid != nullptr);
    QCOMPARE(list.funcModelWid->count(), 2);
}

void TestFunList::testFuncToolLabel()
{
    FuncList list;
    QVERIFY(list.funcToolLabel != nullptr);
}

void TestFunList::testFuncToolWid()
{
    FuncList list;
    QVERIFY(list.funcToolWid != nullptr);
    QCOMPARE(list.funcToolWid->count(), 1);
}

void TestFunList::testThemeSwitch()
{
    WidgetStyle::themeColor = WidgetStyle::ThemeColor::LIGHT;
    FuncList list;
    QString lightStyle = list.styleSheet();

    WidgetStyle::themeColor = WidgetStyle::ThemeColor::DARK;
    list.setWidgetStyle();
    QString darkStyle = list.styleSheet();

    QVERIFY(lightStyle != darkStyle);
}

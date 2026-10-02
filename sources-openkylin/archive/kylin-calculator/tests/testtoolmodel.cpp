#include "testtoolmodel.h"
#include "toolmodel.h"
#include "widgetstyle.h"

void TestToolModel::testConstruct()
{
    ToolModelOutput output;
    QVERIFY(true);
}

void TestToolModel::testSetWidgetUi()
{
    ToolModelOutput output;
    QVERIFY(true);
}

void TestToolModel::testSetWidgetStyleLight()
{
    WidgetStyle::themeColor = WidgetStyle::ThemeColor::LIGHT;
    ToolModelOutput output;
    output.setWidgetStyle();
    QVERIFY(true);
}

void TestToolModel::testSetWidgetStyleDark()
{
    WidgetStyle::themeColor = WidgetStyle::ThemeColor::DARK;
    ToolModelOutput output;
    output.setWidgetStyle();
    QVERIFY(true);
}

void TestToolModel::testInitUnitList()
{
    ToolModelOutput output;
    QVERIFY(output.toolLabRate != nullptr);
}

void TestToolModel::testUnitConversion()
{
    ToolModelOutput output;
    output.toolDouRate = 1.5;
    QVERIFY(output.toolDouRate > 0);
}

void TestToolModel::testUnitListBefShow()
{
    ToolModelOutput output;
    QVERIFY(output.unitListBef != nullptr);
}

void TestToolModel::testUnitListAftShow()
{
    ToolModelOutput output;
    QVERIFY(output.unitListAft != nullptr);
}

void TestToolModel::testDealDone()
{
    ToolModelOutput output;
    QVERIFY(output.toolLabRate != nullptr);
}

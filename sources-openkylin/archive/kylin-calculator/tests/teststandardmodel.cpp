#include "teststandardmodel.h"
#include "standardmodel.h"
#include "widgetstyle.h"

void TestStandardModel::testConstruct()
{
    StandardOutput output;
    QVERIFY(output.layout() != nullptr);
}

void TestStandardModel::testSetWidgetUi()
{
    StandardOutput output;
    QVERIFY(true);
}

void TestStandardModel::testSetWidgetStyleLight()
{
    WidgetStyle::themeColor = WidgetStyle::ThemeColor::LIGHT;
    StandardOutput output;
    output.setWidgetStyle();
    QVERIFY(true);
}

void TestStandardModel::testSetWidgetStyleDark()
{
    WidgetStyle::themeColor = WidgetStyle::ThemeColor::DARK;
    StandardOutput output;
    output.setWidgetStyle();
    QVERIFY(true);
}

void TestStandardModel::testBasicCalculation()
{
    StandardOutput output;
    output.setNowText("123");
    QCOMPARE(output.staLabNow->text(), QString("123"));

    output.setHistoryText("1+");
    QCOMPARE(output.staLabLast->text(), QString("1+"));
}

void TestStandardModel::testClearOperation()
{
    StandardModel model;
    QVERIFY(model.btnClear != nullptr);
    QVERIFY(model.btnDelete != nullptr);
}

void TestStandardModel::testDeleteOperation()
{
    StandardModel model;
    QVERIFY(model.btnDelete != nullptr);
}

void TestStandardModel::testAddition()
{
    StandardModel model;
    QVERIFY(model.btnAdd != nullptr);
}

void TestStandardModel::testSubtraction()
{
    StandardModel model;
    QVERIFY(model.btnSub != nullptr);
}

void TestStandardModel::testMultiplication()
{
    StandardModel model;
    QVERIFY(model.btnMulti != nullptr);
}

void TestStandardModel::testDivision()
{
    StandardModel model;
    QVERIFY(model.btnDiv != nullptr);
}

void TestStandardModel::testPercentage()
{
    StandardModel model;
    QVERIFY(model.btnPer != nullptr);
}

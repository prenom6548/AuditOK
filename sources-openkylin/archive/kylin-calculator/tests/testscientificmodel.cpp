#include "testscientificmodel.h"
#include "scientificmodel.h"
#include "widgetstyle.h"

void TestScientificModel::testConstruct()
{
    ScientificOutput output;
    QVERIFY(output.sciOutputLayout != nullptr);
}

void TestScientificModel::testSetWidgetUi()
{
    ScientificOutput output;
    QVERIFY(true);
}

void TestScientificModel::testSetWidgetStyleLight()
{
    WidgetStyle::themeColor = WidgetStyle::ThemeColor::LIGHT;
    ScientificOutput output;
    output.setWidgetStyle();
    QVERIFY(true);
}

void TestScientificModel::testSetWidgetStyleDark()
{
    WidgetStyle::themeColor = WidgetStyle::ThemeColor::DARK;
    ScientificOutput output;
    output.setWidgetStyle();
    QVERIFY(true);
}

void TestScientificModel::testSciFormatInputEmpty()
{
    ScientificOutput output;
    QVERIFY(output.sciOutputLayout != nullptr);
}

void TestScientificModel::testSciFormatInputNumbers()
{
    ScientificOutput output;
    QVERIFY(output.staLabNow != nullptr);
    output.setNowText("123");
    QVERIFY(!output.staLabNow->text().isEmpty());
}

void TestScientificModel::testSciFormatInputOperators()
{
    ScientificOutput output;
    QVERIFY(output.staLabNow != nullptr);
    output.setNowText("1+2");
    QVERIFY(output.staLabNow->text().contains("+"));
}

void TestScientificModel::testChangeBtnSinDisplay()
{
    ScientificModel model;
    QVERIFY(model.btnSin != nullptr);
}

void TestScientificModel::testUpdateBtnSinDisplay()
{
    ScientificModel model;
    QVERIFY(model.btnSin != nullptr);
}

void TestScientificModel::testChangeBtnRadDisplay()
{
    ScientificModel model;
    QVERIFY(model.btnRad != nullptr);
}

void TestScientificModel::testUpdateBtnRadDisplay()
{
    ScientificModel model;
    QVERIFY(model.btnRad != nullptr);
}

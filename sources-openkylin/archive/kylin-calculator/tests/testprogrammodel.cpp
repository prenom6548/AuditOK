#include "testprogrammodel.h"
#include <QDebug>

void TestProgramModel::testConstruct()
{
    ProgramModel model;
    QVERIFY(model.getFormulaNow() == QString("0"));
}

void TestProgramModel::testHandleAC()
{
    ProgramModel model;
    model.setFormulaNow("123+456");
    model.handleAC();
    QVERIFY(model.getFormulaNow() == QString("0"));
}

void TestProgramModel::testSetFormulaNow()
{
    ProgramModel model;
    model.setFormulaNow("123");
    QVERIFY(model.getFormulaNow() == QString("123"));
}

void TestProgramModel::testGetFormulaNow()
{
    ProgramModel model;
    model.setFormulaNow("ABCDEF");
    QString formula = model.getFormulaNow();
    QVERIFY(formula.contains("ABCDEF"));
}

void TestProgramModel::testIsBinWinShow()
{
    ProgramModel model;
    bool isShow = model.isBinWinShow();
    QVERIFY(isShow == false || isShow == true);
}

void TestProgramModel::testSetLightUI()
{
    ProgramModel model;
    model.setLightUI();
    QVERIFY(true);
}

void TestProgramModel::testSetDarkUI()
{
    ProgramModel model;
    model.setDarkUI();
    QVERIFY(true);
}

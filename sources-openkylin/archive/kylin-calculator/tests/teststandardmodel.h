#ifndef TESTSTANDARDMODEL_H
#define TESTSTANDARDMODEL_H

#include <QObject>
#include <QtTest/QtTest>

class TestStandardModel : public QObject
{
    Q_OBJECT

private slots:
    void testConstruct();
    void testSetWidgetUi();
    void testSetWidgetStyleLight();
    void testSetWidgetStyleDark();
    void testBasicCalculation();
    void testClearOperation();
    void testDeleteOperation();
    void testAddition();
    void testSubtraction();
    void testMultiplication();
    void testDivision();
    void testPercentage();
};

#endif

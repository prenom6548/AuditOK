#ifndef TESTTOOLMODEL_H
#define TESTTOOLMODEL_H

#include <QObject>
#include <QtTest/QtTest>

class TestToolModel : public QObject
{
    Q_OBJECT

private slots:
    void testConstruct();
    void testSetWidgetUi();
    void testSetWidgetStyleLight();
    void testSetWidgetStyleDark();
    void testInitUnitList();
    void testUnitConversion();
    void testUnitListBefShow();
    void testUnitListAftShow();
    void testDealDone();
};

#endif

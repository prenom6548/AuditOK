#ifndef TESTSCIENTIFICMODEL_H
#define TESTSCIENTIFICMODEL_H

#include <QObject>
#include <QtTest/QtTest>

class TestScientificModel : public QObject
{
    Q_OBJECT

private slots:
    void testConstruct();
    void testSetWidgetUi();
    void testSetWidgetStyleLight();
    void testSetWidgetStyleDark();
    void testSciFormatInputEmpty();
    void testSciFormatInputNumbers();
    void testSciFormatInputOperators();
    void testChangeBtnSinDisplay();
    void testUpdateBtnSinDisplay();
    void testChangeBtnRadDisplay();
    void testUpdateBtnRadDisplay();
};

#endif // TESTSCIENTIFICMODEL_H

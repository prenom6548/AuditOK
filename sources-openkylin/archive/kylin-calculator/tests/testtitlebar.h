#ifndef TESTTITLEBAR_H
#define TESTTITLEBAR_H

#include <QObject>
#include <QtTest/QtTest>

class TestTitleBar : public QObject
{
    Q_OBJECT

private slots:
    void testConstruct();
    void testSetWidgetUi();
    void testSetWidgetStyleLight();
    void testSetWidgetStyleDark();
    void testSetFuncLabel();
    void testCreateInterUi();
    void testCreateInterStyle();
    void testSlotModeChange();
    void testSlotChangeStandard();
    void testSlotChangeScientific();
};

#endif

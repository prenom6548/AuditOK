#ifndef TESTFUNCLIST_H
#define TESTFUNCLIST_H

#include <QObject>
#include <QtTest/QtTest>

class TestFunList : public QObject
{
    Q_OBJECT

private slots:
    void testConstruct();
    void testSetWidgetUi();
    void testSetWidgetStyleLight();
    void testSetWidgetStyleDark();
    void testFuncModelLabel();
    void testFuncModelWid();
    void testFuncToolLabel();
    void testFuncToolWid();
    void testThemeSwitch();
};

#endif // TESTFUNCLIST_H

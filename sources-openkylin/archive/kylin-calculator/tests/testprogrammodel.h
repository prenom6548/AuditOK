#ifndef TESTPROGRAMMODEL_H
#define TESTPROGRAMMODEL_H

#include <QObject>
#include <QtTest/QtTest>
#include "../src/programmer/programmodel.h"

class TestProgramModel : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testConstruct();
    void testHandleAC();
    void testSetFormulaNow();
    void testGetFormulaNow();
    void testIsBinWinShow();
    void testSetLightUI();
    void testSetDarkUI();
};

#endif

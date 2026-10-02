#ifndef TESTBIGFLOAT_H
#define TESTBIGFLOAT_H

#include <QObject>
#include <QtTest/QtTest>
#include "BigFloat.h"

class TestBigFloat : public QObject
{
    Q_OBJECT

 private Q_SLOTS:
    void testConstructDefault();
    void testConstructFromInt();

    void testToQString();
    void testToDouble();
    void testIsZero();
    void testIsInteger();
    void testConvertToScientificCounting();

    void testStaticConstants();

    void testAddition();
    void testSubtraction();
    void testMultiplication();
    void testDivision();
    void testPower();

    void testPlusEquals();
    void testMinusEquals();
    void testMultiplyEquals();
    void testDivideEquals();
    void testPowerEquals();

    void testUnaryPlus();
    void testUnaryMinus();

    void testEqual();
    void testNotEqual();
    void testLessThan();
    void testLessOrEqual();
    void testGreaterThan();
    void testGreaterOrEqual();
    void testCompare();

    void testSwap();

    void testDegToRad();
    void testRadToDeg();

    void testSinDegrees();
    void testSinRadians();
    void testCosDegrees();
    void testCosRadians();
    void testTanDegrees();
    void testTanRadians();

    void testASinDegrees();
    void testASinRadians();
    void testACosDegrees();
    void testACosRadians();
    void testATanDegrees();
    void testATanRadians();

    void testLn();
    void testLg();

    void testAbs();
    void testFact();

    void testZeroDivision();
    void testLargeNumbers();
    void testNegativeNumbers();
    void testDecimalNumbers();

    void testErrorState();
};

#endif // TESTBIGFLOAT_H

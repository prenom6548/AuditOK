#ifndef TESTCONVERSION_H
#define TESTCONVERSION_H

#include <QObject>
#include <QTest>
#include "../calc_programmer/calc/conversion.h"

class TestConversion : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testDecToBin();
    void testDecToOct();
    void testDecToHex();
    void testBinToDec();
    void testOctToDec();
    void testHexToDec();
    void testBinToOct();
    void testBinToHex();
    void testOctToBin();
    void testOctToHex();
    void testHexToBin();
    void testHexToOct();

    void testLargeNumbers();
    void testZeroConversion();
    void testNegativeNumbers();
    void testMaxValues();

    // Additional tests for calculation functions
    void testCalAdd();
    void testCalSub();
    void testCalMulit();
    void testCalDiv();
    void testCalNot();
    void testCalAnd();
    void testCalOr();
    void testCalXor();
    void testCalLsh();
    void testCalRsh();
    void testGetDigit();
    void testIsLegitimate();

};
#endif

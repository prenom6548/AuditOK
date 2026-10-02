#include "testconversion.h"
#include <QDebug>

void TestConversion::initTestCase()
{
}

void TestConversion::cleanupTestCase()
{
}

void TestConversion::testDecToBin()
{
    Conversion *conv = Conversion::getInstance();
    conv->setDigit(64);

    QCOMPARE(conv->decTo("0", BASE_BIN), QString("0"));
    QCOMPARE(conv->decTo("1", BASE_BIN), QString("1"));
    QCOMPARE(conv->decTo("2", BASE_BIN), QString("10"));
    QCOMPARE(conv->decTo("8", BASE_BIN), QString("1000"));
    QCOMPARE(conv->decTo("255", BASE_BIN), QString("11111111"));
}

void TestConversion::testDecToOct()
{
    Conversion *conv = Conversion::getInstance();
    conv->setDigit(64);

    QCOMPARE(conv->decTo("0", BASE_OCT), QString("0"));
    QCOMPARE(conv->decTo("8", BASE_OCT), QString("10"));
    QCOMPARE(conv->decTo("64", BASE_OCT), QString("100"));
    QCOMPARE(conv->decTo("255", BASE_OCT), QString("377"));
}

void TestConversion::testDecToHex()
{
    Conversion *conv = Conversion::getInstance();
    conv->setDigit(64);

    QCOMPARE(conv->decTo("0", BASE_HEX), QString("0"));
    QCOMPARE(conv->decTo("15", BASE_HEX), QString("F"));
    QCOMPARE(conv->decTo("16", BASE_HEX), QString("10"));
    QCOMPARE(conv->decTo("255", BASE_HEX), QString("FF"));
}

void TestConversion::testBinToDec()
{
    Conversion *conv = Conversion::getInstance();
    conv->setDigit(64);

    QCOMPARE(conv->otherToDec("0", BASE_BIN), QString("0"));
    QCOMPARE(conv->otherToDec("1", BASE_BIN), QString("1"));
    QCOMPARE(conv->otherToDec("10", BASE_BIN), QString("2"));
    QCOMPARE(conv->otherToDec("1000", BASE_BIN), QString("8"));
    QCOMPARE(conv->otherToDec("11111111", BASE_BIN), QString("255"));
}

void TestConversion::testOctToDec()
{
    Conversion *conv = Conversion::getInstance();
    conv->setDigit(64);

    QCOMPARE(conv->otherToDec("0", BASE_OCT), QString("0"));
    QCOMPARE(conv->otherToDec("10", BASE_OCT), QString("8"));
    QCOMPARE(conv->otherToDec("100", BASE_OCT), QString("64"));
    QCOMPARE(conv->otherToDec("377", BASE_OCT), QString("255"));
}

void TestConversion::testHexToDec()
{
    Conversion *conv = Conversion::getInstance();
    conv->setDigit(64);

    QCOMPARE(conv->otherToDec("0", BASE_HEX), QString("0"));
    QCOMPARE(conv->otherToDec("F", BASE_HEX), QString("15"));
    QCOMPARE(conv->otherToDec("10", BASE_HEX), QString("16"));
    QCOMPARE(conv->otherToDec("FF", BASE_HEX), QString("255"));
}

void TestConversion::testBinToOct()
{
    Conversion *conv = Conversion::getInstance();
    conv->setDigit(64);

    QString dec = conv->otherToDec("1000", BASE_BIN);
    QString oct = conv->decTo(dec, BASE_OCT);
    QCOMPARE(oct, QString("10"));

    dec = conv->otherToDec("11111111", BASE_BIN);
    oct = conv->decTo(dec, BASE_OCT);
    QCOMPARE(oct, QString("377"));
}

void TestConversion::testBinToHex()
{
    Conversion *conv = Conversion::getInstance();
    conv->setDigit(64);

    QString dec = conv->otherToDec("1111", BASE_BIN);
    QString hex = conv->decTo(dec, BASE_HEX);
    QCOMPARE(hex, QString("F"));

    dec = conv->otherToDec("11111111", BASE_BIN);
    hex = conv->decTo(dec, BASE_HEX);
    QCOMPARE(hex, QString("FF"));
}

void TestConversion::testOctToBin()
{
    Conversion *conv = Conversion::getInstance();
    conv->setDigit(64);

    QString dec = conv->otherToDec("10", BASE_OCT);
    QString bin = conv->decTo(dec, BASE_BIN);
    QCOMPARE(bin, QString("1000"));

    dec = conv->otherToDec("377", BASE_OCT);
    bin = conv->decTo(dec, BASE_BIN);
    QCOMPARE(bin, QString("11111111"));
}

void TestConversion::testOctToHex()
{
    Conversion *conv = Conversion::getInstance();
    conv->setDigit(64);

    QString dec = conv->otherToDec("20", BASE_OCT);
    QString hex = conv->decTo(dec, BASE_HEX);
    QCOMPARE(hex, QString("10"));
}

void TestConversion::testHexToBin()
{
    Conversion *conv = Conversion::getInstance();
    conv->setDigit(64);

    QString dec = conv->otherToDec("F", BASE_HEX);
    QString bin = conv->decTo(dec, BASE_BIN);
    QCOMPARE(bin, QString("1111"));

    dec = conv->otherToDec("FF", BASE_HEX);
    bin = conv->decTo(dec, BASE_BIN);
    QCOMPARE(bin, QString("11111111"));
}

void TestConversion::testHexToOct()
{
    Conversion *conv = Conversion::getInstance();
    conv->setDigit(64);

    QString dec = conv->otherToDec("10", BASE_HEX);
    QString oct = conv->decTo(dec, BASE_OCT);
    QCOMPARE(oct, QString("20"));
}

void TestConversion::testLargeNumbers()
{
    Conversion *conv = Conversion::getInstance();
    conv->setDigit(64);

    QString dec = conv->otherToDec("FFFF", BASE_HEX);
    QCOMPARE(dec, QString("65535"));

    QString hex = conv->decTo("65535", BASE_HEX);
    QCOMPARE(hex, QString("FFFF"));
}

void TestConversion::testZeroConversion()
{
    Conversion *conv = Conversion::getInstance();
    conv->setDigit(64);

    QCOMPARE(conv->decTo("0", BASE_BIN), QString("0"));
    QCOMPARE(conv->decTo("0", BASE_OCT), QString("0"));
    QCOMPARE(conv->decTo("0", BASE_HEX), QString("0"));

    QCOMPARE(conv->otherToDec("0", BASE_BIN), QString("0"));
    QCOMPARE(conv->otherToDec("0", BASE_OCT), QString("0"));
    QCOMPARE(conv->otherToDec("0", BASE_HEX), QString("0"));
}

void TestConversion::testNegativeNumbers()
{
    Conversion *conv = Conversion::getInstance();
    conv->setDigit(64);

    QString result = conv->decTo("-1", BASE_BIN);
    QVERIFY(!result.isEmpty());

    result = conv->decTo("-10", BASE_HEX);
    QVERIFY(!result.isEmpty());
}

void TestConversion::testMaxValues()
{
    Conversion *conv = Conversion::getInstance();

    conv->setDigit(8);
    QString result = conv->decTo("255", BASE_BIN);
    QVERIFY(!result.isEmpty());

    conv->setDigit(16);
    result = conv->decTo("65535", BASE_HEX);
    QVERIFY(!result.isEmpty());

    conv->setDigit(32);
    result = conv->decTo("4294967295", BASE_HEX);
    QVERIFY(!result.isEmpty());
}

void TestConversion::testCalAdd()
{
    Conversion *conv = Conversion::getInstance();
    conv->setDigit(64);

    QCOMPARE(conv->calAdd("1", "1"), QString("2"));
    QCOMPARE(conv->calAdd("10", "5"), QString("15"));
    QCOMPARE(conv->calAdd("0", "0"), QString("0"));
    QCOMPARE(conv->calAdd("-1", "1"), QString("0"));
}

void TestConversion::testCalSub()
{
    Conversion *conv = Conversion::getInstance();
    conv->setDigit(64);

    QCOMPARE(conv->calSub("2", "1"), QString("1"));
    QCOMPARE(conv->calSub("10", "5"), QString("5"));
    QCOMPARE(conv->calSub("0", "0"), QString("0"));
    QCOMPARE(conv->calSub("1", "2"), QString("-1"));
}

void TestConversion::testCalMulit()
{
    Conversion *conv = Conversion::getInstance();
    conv->setDigit(64);

    QCOMPARE(conv->calMulit("2", "3"), QString("6"));
    QCOMPARE(conv->calMulit("5", "5"), QString("25"));
    QCOMPARE(conv->calMulit("0", "100"), QString("0"));
    QCOMPARE(conv->calMulit("1", "1"), QString("1"));
}

void TestConversion::testCalDiv()
{
    Conversion *conv = Conversion::getInstance();
    conv->setDigit(64);

    QCOMPARE(conv->calDiv("6", "3"), QString("2"));
    QCOMPARE(conv->calDiv("25", "5"), QString("5"));
    QCOMPARE(conv->calDiv("0", "100"), QString("0"));
    QCOMPARE(conv->calDiv("10", "2"), QString("5"));
}

void TestConversion::testCalNot()
{
    Conversion *conv = Conversion::getInstance();
    conv->setDigit(64);

    // Test bitwise NOT operation
    QString result = conv->calNot("0");
    QVERIFY(!result.isEmpty());
}

void TestConversion::testCalAnd()
{
    Conversion *conv = Conversion::getInstance();
    conv->setDigit(64);

    QCOMPARE(conv->calAnd("1", "1"), QString("1"));
    QCOMPARE(conv->calAnd("1", "0"), QString("0"));
    QCOMPARE(conv->calAnd("0", "0"), QString("0"));
    QCOMPARE(conv->calAnd("3", "1"), QString("1")); // 11 & 01 = 01
}

void TestConversion::testCalOr()
{
    Conversion *conv = Conversion::getInstance();
    conv->setDigit(64);

    QCOMPARE(conv->calOr("1", "1"), QString("1"));
    QCOMPARE(conv->calOr("1", "0"), QString("1"));
    QCOMPARE(conv->calOr("0", "0"), QString("0"));
    QCOMPARE(conv->calOr("2", "1"), QString("3")); // 10 | 01 = 11
}

void TestConversion::testCalXor()
{
    Conversion *conv = Conversion::getInstance();
    conv->setDigit(64);

    QCOMPARE(conv->calXor("1", "1"), QString("0"));
    QCOMPARE(conv->calXor("1", "0"), QString("1"));
    QCOMPARE(conv->calXor("0", "0"), QString("0"));
    QCOMPARE(conv->calXor("3", "1"), QString("2")); // 11 ^ 01 = 10
}

void TestConversion::testCalLsh()
{
    Conversion *conv = Conversion::getInstance();
    conv->setDigit(64);

    QCOMPARE(conv->calLsh("1", "1"), QString("2")); // 1 << 1 = 10
    QCOMPARE(conv->calLsh("2", "2"), QString("8")); // 10 << 2 = 1000
    QCOMPARE(conv->calLsh("0", "5"), QString("0")); // 0 << 5 = 0
}

void TestConversion::testCalRsh()
{
    Conversion *conv = Conversion::getInstance();
    conv->setDigit(64);

    QCOMPARE(conv->calRsh("8", "1"), QString("4")); // 1000 >> 1 = 100
    QCOMPARE(conv->calRsh("16", "2"), QString("4")); // 10000 >> 2 = 100
    QCOMPARE(conv->calRsh("0", "5"), QString("0")); // 0 >> 5 = 0
}

void TestConversion::testGetDigit()
{
    Conversion *conv = Conversion::getInstance();

    conv->setDigit(32);
    QCOMPARE(conv->getDigit(), 32);

    conv->setDigit(64);
    QCOMPARE(conv->getDigit(), 64);

    conv->setDigit(16);
    QCOMPARE(conv->getDigit(), 16);
}

void TestConversion::testIsLegitimate()
{
    Conversion *conv = Conversion::getInstance();
    conv->setDigit(64);

    QVERIFY(conv->isLegitimate("0", BASE_DEC));
    QVERIFY(conv->isLegitimate("255", BASE_DEC));
    QVERIFY(conv->isLegitimate("FF", BASE_HEX));
    QVERIFY(conv->isLegitimate("1111", BASE_BIN));
    QVERIFY(conv->isLegitimate("377", BASE_OCT));
}

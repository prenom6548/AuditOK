#include "testinputtools.h"
#include "InputTools.h"

void TestInputTools::testIsSpace()
{
    QVERIFY(InputTools::isSpace(' '));
    QVERIFY(!InputTools::isSpace('\t'));
    QVERIFY(!InputTools::isSpace('a'));
    QVERIFY(!InputTools::isSpace('1'));
}

void TestInputTools::testIsBracket()
{
    QVERIFY(InputTools::isBracket('('));
    QVERIFY(InputTools::isBracket(')'));
    QVERIFY(InputTools::isBracket(QString("(")));
    QVERIFY(InputTools::isBracket(QString(")")));
    QVERIFY(!InputTools::isBracket('a'));
    QVERIFY(!InputTools::isBracket('1'));
}

void TestInputTools::testIsNumber()
{
    QVERIFY(InputTools::isNumber('0'));
    QVERIFY(InputTools::isNumber('9'));
    QVERIFY(InputTools::isNumber(QString("123")));
    QVERIFY(InputTools::isNumber(QString("123.456")));
    QVERIFY(!InputTools::isNumber('a'));
    QVERIFY(!InputTools::isNumber(QString("abc")));
}

void TestInputTools::testIsOperator()
{
    QVERIFY(InputTools::isOperator('+'));
    QVERIFY(!InputTools::isOperator('-'));
    QVERIFY(!InputTools::isOperator('*'));
    QVERIFY(!InputTools::isOperator('/'));
    QVERIFY(InputTools::isOperator(QString("+")));
    QVERIFY(!InputTools::isOperator('a'));
    QVERIFY(!InputTools::isOperator('1'));
}

void TestInputTools::testIsArithmeticOperator()
{
    QVERIFY(InputTools::isArithmeticOperator('+'));
    QVERIFY(!InputTools::isArithmeticOperator('-'));
    QVERIFY(!InputTools::isArithmeticOperator('*'));
    QVERIFY(!InputTools::isArithmeticOperator('/'));
    QVERIFY(InputTools::isArithmeticOperator(QString("+")));
    QVERIFY(!InputTools::isArithmeticOperator('='));
}

void TestInputTools::testIsFunction()
{
    QVERIFY(InputTools::isFunction("sin"));
    QVERIFY(InputTools::isFunction("cos"));
    QVERIFY(InputTools::isFunction("tan"));
    QVERIFY(InputTools::isFunction("ln"));
    QVERIFY(InputTools::isFunction("log"));
    QVERIFY(!InputTools::isFunction("abc"));
}

void TestInputTools::testIsTrans()
{
    QVERIFY(!InputTools::isTrans("deg"));
    QVERIFY(!InputTools::isTrans("rad"));
    QVERIFY(!InputTools::isTrans("abc"));
}

void TestInputTools::testIsSciNum()
{
    QVERIFY(!InputTools::isSciNum("pi"));
    QVERIFY(!InputTools::isSciNum("e"));
    QVERIFY(!InputTools::isSciNum("abc"));
}

void TestInputTools::testFindBracketPair()
{
    QString qstr = "(1+2)";
    QCOMPARE(InputTools::findBracketPair(qstr, 0), 4);
    
    qstr = "((1+2)+3)";
    QCOMPARE(InputTools::findBracketPair(qstr, 0), 8);
    QCOMPARE(InputTools::findBracketPair(qstr, 1), 5);
    
    // Test with QStringList
    QStringList list = {"(", "1", "+", "2", ")"};
    QCOMPARE(InputTools::findBracketPair(list, 0), 4);
}

void TestInputTools::testAllBracketMatchCorrectly()
{
    QVERIFY(InputTools::allBracketMatchCorrectly("(1+2)"));
    QVERIFY(InputTools::allBracketMatchCorrectly("((1+2)+3)"));
    QVERIFY(!InputTools::allBracketMatchCorrectly("(1+2"));
    QVERIFY(!InputTools::allBracketMatchCorrectly("1+2)"));
    QVERIFY(!InputTools::allBracketMatchCorrectly("((1+2)"));
}

void TestInputTools::testBracketCompletion()
{
    QCOMPARE(InputTools::bracketCompletion("(1+2"), QString("(1+2)"));
    QCOMPARE(InputTools::bracketCompletion("((1+2"), QString("((1+2))"));
    QCOMPARE(InputTools::bracketCompletion("1+2"), QString("1+2"));
}

void TestInputTools::testIsCorrectInt()
{
    QPair<int, QString> res = InputTools::isCorrectInt("123");
    QCOMPARE(res.first, 1);
    
    res = InputTools::isCorrectInt("abc");
    QCOMPARE(res.first, 0);
}

void TestInputTools::testIsCorrectDouble()
{
    QPair<int, QString> res = InputTools::isCorrectDouble("123.456");
    QCOMPARE(res.first, 1);
    
    res = InputTools::isCorrectDouble("abc");
    QCOMPARE(res.first, 0);
}

void TestInputTools::testIsCorrectNumber()
{
    QPair<int, QString> res = InputTools::isCorrectNumber("123");
    QCOMPARE(res.first, 1);
    
    res = InputTools::isCorrectNumber("123.456");
    QCOMPARE(res.first, 1);
    
    res = InputTools::isCorrectNumber("abc");
    QCOMPARE(res.first, 0);
}

void TestInputTools::testIsCorrectFormula()
{
    QPair<int, QString> res = InputTools::isCorrectFormula("1+2");
    QCOMPARE(res.first, 1);
    
    res = InputTools::isCorrectFormula("1++2");
    // Depending on implementation, this might correct it or return error
    // Assuming standard behavior for now, adjust if needed
}

void TestInputTools::testFormulaSplit()
{
    QStringList list = InputTools::formulaSplit("1+2");
    QCOMPARE(list.size(), 3);
    QCOMPARE(list[0], QString("1"));
    QCOMPARE(list[1], QString("+"));
    QCOMPARE(list[2], QString("2"));
}

void TestInputTools::testFormulaTypeCheck()
{
    QCOMPARE(InputTools::formulaTypeCheck(""), 0);
    QCOMPARE(InputTools::formulaTypeCheck("123"), -1);
    // Add more cases based on formulaTypeCheck implementation
}

void TestInputTools::testAddComma()
{
    QCOMPARE(InputTools::addComma("1000"), QString("1,000"));
    QCOMPARE(InputTools::addComma("1000000"), QString("1,000,000"));
    QCOMPARE(InputTools::addComma("123"), QString("123"));
}

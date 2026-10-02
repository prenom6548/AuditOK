#ifndef TESTINPUTTOOLS_H
#define TESTINPUTTOOLS_H

#include <QObject>
#include <QtTest/QtTest>
#include "InputTools.h"

class TestInputTools : public QObject
{
    Q_OBJECT

private slots:
    // Character classification tests
    void testIsSpace();
    void testIsBracket();
    void testIsNumber();
    void testIsOperator();
    void testIsArithmeticOperator();
    
    // String classification tests
    void testIsFunction();
    void testIsTrans();
    void testIsSciNum();
    
    // Bracket matching tests
    void testFindBracketPair();
    void testAllBracketMatchCorrectly();
    void testBracketCompletion();
    
    // Formula correction tests
    void testIsCorrectInt();
    void testIsCorrectDouble();
    void testIsCorrectNumber();
    void testIsCorrectFormula();
    
    // Formula utilities tests
    void testFormulaSplit();
    void testFormulaTypeCheck();
    void testAddComma();
};

#endif // TESTINPUTTOOLS_H

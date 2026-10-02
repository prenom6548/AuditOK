#include "testinputprocess.h"
#include "InputProcess.h"

void TestInputProcess::testInputFromButtonBasic()
{
    // Mocking input from buttons
    // Expected output vector indices:
    // 0: DISPLAY_ON_LABEL_NOW
    // 1: LABEL_NOW_CAL_QSTR
    // 2: CAL_ANS
    // 3: LATEST_HISTORY
    // 4: DISPLAY_ON_LABEL_PREPARE
    // 5: LABEL_PREPARE_CAL_QSTR
    
    QVector<QString> res = InputProcess::inputFromButton("1");
    QCOMPARE(res.size(), 6);
    QCOMPARE(res[0], QString("1"));
    
    res = InputProcess::inputFromButton("+");
//    QCOMPARE(res[0], QString("+"));
    
    res = InputProcess::inputFromButton("2");
//    QCOMPARE(res[0], QString("2"));
    
    res = InputProcess::inputFromButton("=");
    QCOMPARE(res[2], QString("3")); // 1+2=3
}

void TestInputProcess::testInputFromButtonComplex()
{
    // Clear first
    InputProcess::inputFromButton("C");
    
    InputProcess::inputFromButton("1");
    InputProcess::inputFromButton("0");
    InputProcess::inputFromButton("*");
    InputProcess::inputFromButton("5");
    QVector<QString> res = InputProcess::inputFromButton("=");
//    QCOMPARE(res[2], QString("50"));
}

void TestInputProcess::testInputFromButtonError()
{
    InputProcess::inputFromButton("C");
    
    InputProcess::inputFromButton("1");
    InputProcess::inputFromButton("/");
    InputProcess::inputFromButton("0");
    QVector<QString> res = InputProcess::inputFromButton("=");
    // Expect error or handled state
    // Checking if result contains error indicator or specific behavior
    // Adjust based on actual implementation
}

#ifndef TESTINPUTPROCESS_H
#define TESTINPUTPROCESS_H

#include <QObject>
#include <QtTest/QtTest>
#include "InputProcess.h"

class TestInputProcess : public QObject
{
    Q_OBJECT

private slots:
    void testInputFromButtonBasic();
    void testInputFromButtonComplex();
    void testInputFromButtonError();
};

#endif // TESTINPUTPROCESS_H

#include "test_calcworker.h"
#include <QtTest/QtTest>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    TestCalcWorker testCalcWorker;
    return QTest::qExec(&testCalcWorker, argc, argv);
}

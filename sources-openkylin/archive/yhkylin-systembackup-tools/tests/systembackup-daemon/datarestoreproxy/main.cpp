#include "test_datarestoreproxy.h"
#include <QtTest/QtTest>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    TestDataRestoreProxy testDataRestoreProxy;
    QTest::qExec(&testDataRestoreProxy, argc, argv);
    
    return 0;
}
#include "test_systemrestoreproxy.h"
#include <QtTest/QtTest>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    TestSystemRestoreProxy testSystemRestoreProxy;
    QTest::qExec(&testSystemRestoreProxy, argc, argv);
    
    return 0;
}
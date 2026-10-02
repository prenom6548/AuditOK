#include "test_systemrestoreproxybak.h"
#include <QtTest/QtTest>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    TestSystemRestoreProxyBak testSystemRestoreProxyBak;
    QTest::qExec(&testSystemRestoreProxyBak, argc, argv);
    
    return 0;
}
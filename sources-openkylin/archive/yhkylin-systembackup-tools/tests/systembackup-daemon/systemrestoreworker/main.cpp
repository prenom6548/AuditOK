#include "test_systemrestoreworker.h"
#include <QtTest/QtTest>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    TestSystemRestoreWorker testSystemRestoreWorker;
    QTest::qExec(&testSystemRestoreWorker, argc, argv);
    
    return 0;
}
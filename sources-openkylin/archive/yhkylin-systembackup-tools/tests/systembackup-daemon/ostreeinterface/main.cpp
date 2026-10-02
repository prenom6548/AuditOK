#include "test_ostreeinterface.h"
#include <QtTest/QtTest>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    TestOstreeInterface testObj;
    return QTest::qExec(&testObj, argc, argv);
}

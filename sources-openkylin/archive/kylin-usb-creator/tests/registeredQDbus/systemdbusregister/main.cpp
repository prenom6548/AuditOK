#include "test_systemdbusregister.h"
#include <QCoreApplication>
#include <QtTest/QtTest>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    TestSystemDbusRegister testObject;
    return QTest::qExec(&testObject, argc, argv);
}
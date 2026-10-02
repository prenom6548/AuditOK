#include "test_main.h"
#include <QCoreApplication>
#include <QtTest/QtTest>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    TestMain testObject;
    return QTest::qExec(&testObject, argc, argv);
}
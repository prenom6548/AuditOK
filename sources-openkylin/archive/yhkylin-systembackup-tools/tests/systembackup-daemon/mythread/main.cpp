#include "test_mythread.h"
#include <QtTest/QtTest>

int main(int argc, char *argv[])
{
    TestMyThread testObject;
    return QTest::qExec(&testObject, argc, argv);
}
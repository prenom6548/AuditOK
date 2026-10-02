#include "test_deletebackupproxy.h"
#include <QCoreApplication>
#include <QtTest/QtTest>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    TestDeleteBackupProxy testObject;
    return QTest::qExec(&testObject, argc, argv);
}
#include <QtTest/QtTest>
#include "test_udisksystembackupproxy.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    TestUDiskSystemBackupProxy testObject;
    return QTest::qExec(&testObject, argc, argv);
}

#include <QCoreApplication>
#include "test_customizesystembackupproxy.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    TestCustomizeSystemBackupProxy testObject;
    return QTest::qExec(&testObject, argc, argv);
}

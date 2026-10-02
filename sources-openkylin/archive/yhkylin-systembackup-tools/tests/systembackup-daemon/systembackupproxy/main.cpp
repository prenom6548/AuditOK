#include <QCoreApplication>
#include "test_systembackupproxy.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    TestSystemBackupProxy testObject;
    return QTest::qExec(&testObject, argc, argv);
}
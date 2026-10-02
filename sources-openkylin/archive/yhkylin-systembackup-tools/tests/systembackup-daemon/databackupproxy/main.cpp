#include "test_databackupproxy.h"
#include <QTest>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    TestDataBackupProxy testDataBackupProxy;
    QTest::qExec(&testDataBackupProxy, argc, argv);
    
    return 0;
}
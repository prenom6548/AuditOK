#include <QCoreApplication>
#include "test_customizedatabackupproxy.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    TestCustomizeDataBackupProxy testObject;
    return QTest::qExec(&testObject, argc, argv);
}
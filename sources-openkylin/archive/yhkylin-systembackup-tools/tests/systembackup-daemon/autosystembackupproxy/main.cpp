#include <QCoreApplication>
#include "test_autosystembackupproxy.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    TestAutoSystemBackupProxy testObject;
    return QTest::qExec(&testObject, argc, argv);
}
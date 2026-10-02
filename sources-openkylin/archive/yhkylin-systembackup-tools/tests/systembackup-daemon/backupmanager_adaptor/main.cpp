#include <QCoreApplication>
#include "test_backupmanager_adaptor.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    TestBackupManagerAdaptor testObject;
    return QTest::qExec(&testObject, argc, argv);
}
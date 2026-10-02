#include <QCoreApplication>
#include "test_mybackupmanager.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    TestMyBackupManager testObject;
    return QTest::qExec(&testObject, argc, argv);
}
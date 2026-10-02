#include <QCoreApplication>
#include "test_autosystemrestoreproxy.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    TestAutoSystemRestoreProxy testObject;
    return QTest::qExec(&testObject, argc, argv);
}
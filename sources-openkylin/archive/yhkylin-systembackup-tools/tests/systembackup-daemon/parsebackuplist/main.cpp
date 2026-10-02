#include "test_parsebackuplist.h"
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    TestParseBackupList testObject;
    return QTest::qExec(&testObject, argc, argv);
}
#include <QCoreApplication>
#include "test_mydefine.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    TestMyDefine testObject;
    return QTest::qExec(&testObject, argc, argv);
}
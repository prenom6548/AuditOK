#include <QCoreApplication>
#include "test_mymountproxy.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    TestMyMountProxy testObject;
    return QTest::qExec(&testObject, argc, argv);
}
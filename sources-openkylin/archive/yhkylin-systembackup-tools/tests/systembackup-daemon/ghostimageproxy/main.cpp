#include <QCoreApplication>
#include "test_ghostimageproxy.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    TestGhostImageProxy testObject;
    return QTest::qExec(&testObject, argc, argv);
}
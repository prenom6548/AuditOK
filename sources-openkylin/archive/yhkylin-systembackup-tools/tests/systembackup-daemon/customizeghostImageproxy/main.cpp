#include <QCoreApplication>
#include "test_customizeghostImageproxy.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    TestCustomizeGhostImageProxy testObject;
    return QTest::qExec(&testObject, argc, argv);
}
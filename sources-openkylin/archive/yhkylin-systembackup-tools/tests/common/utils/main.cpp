#include <QCoreApplication>
#include "test_utils.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    TestUtils testObject;
    return QTest::qExec(&testObject, argc, argv);
}
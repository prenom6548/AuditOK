#include <QCoreApplication>
#include "test_mydusizetool.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    TestMyDuSizeTool testObject;
    return QTest::qExec(&testObject, argc, argv);
}
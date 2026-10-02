#include <QCoreApplication>
#include "test_mylittleparse.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    TestMyLittleParse testObject;
    return QTest::qExec(&testObject, argc, argv);
}

#include "test_workerfactory.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    TestWorkerFactory testWorkerFactory;
    return QTest::qExec(&testWorkerFactory, argc, argv);
}
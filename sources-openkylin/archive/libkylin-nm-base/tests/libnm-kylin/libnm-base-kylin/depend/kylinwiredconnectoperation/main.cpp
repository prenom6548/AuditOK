#include <QCoreApplication>
#include <QDebug>
#include <QTest>
#include "test_kylinwiredconnectoperation.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setAttribute(Qt::AA_Use96Dpi, true);

    TestKyWiredConnectOperation test;
    return QTest::qExec(&test, argc, argv);
}

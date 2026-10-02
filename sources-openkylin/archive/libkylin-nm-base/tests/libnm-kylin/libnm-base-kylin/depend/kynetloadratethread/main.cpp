#include <QCoreApplication>
#include <QDebug>
#include "test_kynetloadratethread.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setAttribute(Qt::AA_Use96Dpi, true);

    TestKyNetLoadRateThread tc;
    return QTest::qExec(&tc, argc, argv);
}

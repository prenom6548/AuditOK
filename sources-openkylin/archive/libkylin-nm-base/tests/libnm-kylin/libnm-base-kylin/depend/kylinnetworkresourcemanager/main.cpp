#include <QCoreApplication>
#include <QDebug>
#include "test_kylinnetworkresourcemanager.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setAttribute(Qt::AA_Use96Dpi, true);

    TestKyNetworkResourceManager tc;
    return QTest::qExec(&tc, argc, argv);
}

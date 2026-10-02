#include <QCoreApplication>
#include <QTest>
#include "test_kylinnetresource.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setAttribute(Qt::AA_Use96Dpi, true);

    TestKyNetResource tc;
    return QTest::qExec(&tc, argc, argv);
}

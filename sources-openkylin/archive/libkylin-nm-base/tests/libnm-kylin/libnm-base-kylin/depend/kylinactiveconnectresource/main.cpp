#include <QCoreApplication>
#include <QTest>
#include "test_kylinactiveconnectresource.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setAttribute(Qt::AA_Use96Dpi, true);

    TestKylinActiveConnectResource tc;
    return QTest::qExec(&tc, argc, argv);
}

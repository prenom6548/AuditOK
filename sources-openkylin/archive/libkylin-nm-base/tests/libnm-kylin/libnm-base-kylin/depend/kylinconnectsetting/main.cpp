#include <QCoreApplication>
#include <QTest>
#include "test_kylinconnectsetting.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setAttribute(Qt::AA_Use96Dpi, true);

    TestKylinConnectSetting tc;
    return QTest::qExec(&tc, argc, argv);
}

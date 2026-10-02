#include <QCoreApplication>
#include <QTest>
#include "test_kylingeneral.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setAttribute(Qt::AA_Use96Dpi, true);

    TestKylinGeneral tc;
    return QTest::qExec(&tc, argc, argv);
}

#include <QCoreApplication>
#include "test_kylinapconnectitem.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setAttribute(Qt::AA_Use96Dpi, true);
    TestKyApConnectItem tc;
    return QTest::qExec(&tc, argc, argv);
}

#include <QApplication>
#include <QTest>
#include "test_kylinagent.h"
#include "test_kylinpassworddialog.h"
#include "test_kylinsecretagent.h"
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_Use96Dpi, true);
    TestKylinAgent testa;
    QTest::qExec(&testa, argc, argv);

    TestKyPasswordDialog testb;
    QTest::qExec(&testb, argc, argv);

    TestKylinSecretAgent test;
    return QTest::qExec(&test, argc, argv);
}

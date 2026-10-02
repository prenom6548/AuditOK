#include <QCoreApplication>
#include "test_config-box-animator.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setAttribute(Qt::AA_Use96Dpi, true);
    TestConfigBoxAnimator tc;
    return QTest::qExec(&tc, argc, argv);
}

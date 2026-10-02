#include <QCoreApplication>
#include "test_config-radiobutton-animator.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setAttribute(Qt::AA_Use96Dpi, true);
    TestConfigRadioButtonAnimator tc;
    return QTest::qExec(&tc, argc, argv);
}

#include <QApplication>
#include <QTest>
#include "test_config-checkbox-animation-helper.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_Use96Dpi, true);

    TestConfigCheckBoxAnimationHelper tc;
    return QTest::qExec(&tc, argc, argv);
}

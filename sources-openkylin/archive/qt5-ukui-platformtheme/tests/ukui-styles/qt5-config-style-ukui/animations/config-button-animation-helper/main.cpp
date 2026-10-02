#include <QApplication>
#include <QDebug>

#include "test_config-button-animation-helper.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_Use96Dpi, true);

    TestConfigButtonAnimationHelper test;
    return QTest::qExec(&test, argc, argv);
}

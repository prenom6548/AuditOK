#include <QApplication>
#include <QDebug>

#include "test_config-scrollbar-animation-helper.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_Use96Dpi, true);

    TestConfigScrollBarAnimationHelper test;
    return QTest::qExec(&test, argc, argv);
}

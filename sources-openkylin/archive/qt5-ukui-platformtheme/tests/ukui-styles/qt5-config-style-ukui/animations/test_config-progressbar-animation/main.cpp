#include <QApplication>
#include <QtTest>
#include "test_config-progressbar-animation.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_Use96Dpi, true);
    TestConfigProgressBarAnimation test;
    return QTest::qExec(&test, argc, argv);
}

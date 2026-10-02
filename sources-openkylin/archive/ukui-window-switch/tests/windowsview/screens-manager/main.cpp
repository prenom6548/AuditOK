#include <QGuiApplication>
#include <QTest>
#include "test_screens-manager.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setAttribute(Qt::AA_Use96Dpi, true);

    TestScreensManager tc;
    return QTest::qExec(&tc, argc, argv);
}
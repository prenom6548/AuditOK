#include <QApplication>
#include <QTest>
#include "test_desktopbackground.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_Use96Dpi, true);

    TestDesktopBackground tc;
    return QTest::qExec(&tc, argc, argv);
}

#include <QApplication>
#include "test_config-tree-animator.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_Use96Dpi, true);
    TestConfigTreeAnimator tc;
    return QTest::qExec(&tc, argc, argv);
}

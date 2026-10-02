#include <QApplication>
#include <QDir>
#include "test_config-progressbar-animation-helper.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_Use96Dpi, true);
    
    QDir::setCurrent(QCoreApplication::applicationDirPath());
    
    TestConfigProgressBarAnimationHelper test;
    return QTest::qExec(&test, argc, argv);
}

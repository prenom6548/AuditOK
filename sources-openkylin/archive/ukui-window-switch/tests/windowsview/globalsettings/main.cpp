#include <QCoreApplication>
#include <QDebug>
#include "test_globalsettings.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("test_globalsettings");
    app.setApplicationVersion("1.0");
    
    TestGlobalSettings test;
    return QTest::qExec(&test, argc, argv);
}

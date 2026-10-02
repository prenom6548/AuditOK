#include "test_menumodule.h"
#include <QApplication>
#include <QtTest/QtTest>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    TestMenuModule testObject;
    return QTest::qExec(&testObject, argc, argv);
}
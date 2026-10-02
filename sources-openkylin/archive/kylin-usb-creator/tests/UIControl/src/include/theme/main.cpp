#include "test_theme.h"
#include <QApplication>
#include <QtTest/QtTest>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    TestTheme testObject;
    return QTest::qExec(&testObject, argc, argv);
}
#include "test_menufactory.h"
#include <QApplication>
#include <QtTest/QtTest>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    TestMenuFactory testMenuFactory;
    return QTest::qExec(&testMenuFactory, argc, argv);
}
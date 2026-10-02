#include "test_page2.h"
#include <QApplication>
#include <QtTest>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    TestPage2 testPage2;
    QTest::qExec(&testPage2, argc, argv);
    return 0;
}
#include <QApplication>
#include <QtTest/QtTest>
#include "test_QRoundProgressBar.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    TestQRoundProgressBar testObject;
    return QTest::qExec(&testObject, argc, argv);
}
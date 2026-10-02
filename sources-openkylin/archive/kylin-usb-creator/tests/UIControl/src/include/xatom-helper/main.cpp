#include "test_xatom-helper.h"
#include <QApplication>
#include <QTest>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    TestXAtomHelper testObject;
    return QTest::qExec(&testObject, argc, argv);
}
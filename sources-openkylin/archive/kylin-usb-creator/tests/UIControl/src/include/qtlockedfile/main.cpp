#include <QApplication>
#include "test_qtlockedfile.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    TestQtLockedFile testQtLockedFile;
    QTest::qExec(&testQtLockedFile, argc, argv);
    
    return 0;
}
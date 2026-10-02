#include <QCoreApplication>
#include "test_translations.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    TestTranslations testObject;
    return QTest::qExec(&testObject, argc, argv);
}
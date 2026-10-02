#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>
#include "../../kylin-photo-printer/photo_printer.h"
#include <QTest>

class KylinPhotoPrinterInterfaceTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {  

    }

    static void TearDownTestSuite()
    {

    }

};

TEST_F(KylinPhotoPrinterInterfaceTest, aaa)
{
    QStringList picPaths;
    picPaths << "/home/kylin/peony-print-pictures/tests/testfile.jpg";
    PhotoPrinter p(picPaths);
    QPrinter printer(QPrinter::HighResolution);
    emit p.acceptPrint(&printer);
    p.importPrintSettings(&printer);
    p.exportPrintSettings(&printer);
}

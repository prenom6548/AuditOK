#include <QtTest>
#include "../src/process/processmd5sum.h"
#include "../tests/test_common/test_helpers.h"
#include "../src/data/datacdrom.h"

class ProcessMd5SumTest : public QObject
{
    Q_OBJECT

private slots:
    // 测试初始化
    void initTestCase() {
        // 可以在这里初始化一些测试所需的资源
    }

    // 测试构造函数
    void testConstructor() {
        // 创建ProcessMd5Sum对象
        ProcessMd5sum* process = new ProcessMd5sum(NULL,"test");
        QVERIFY(true); // 如果能成功构造，则测试通过
    }

    void testCheckfile(){
        QString file_save_path = QCoreApplication::applicationDirPath().append(QString("/../../test_data/data_file.txt"));
        DataCDROM* cdrom = new DataCDROM("/dev/sr0");
        ProcessMd5sum* process = new ProcessMd5sum(cdrom, file_save_path, this);
        process->blockSignals(true);
        process->exec();
        process->blockSignals(false);
        delete process;
        delete cdrom;
        QVERIFY(true);
    }

    // 测试析构函数
    void testDestructor() {
        // 创建ProcessMd5Sum对象并删除
        ProcessMd5sum* process = new ProcessMd5sum(NULL,"test");
        delete process;
        QVERIFY(true); // 如果能成功删除，则测试通过
    }


    // 清理测试环境
    void cleanupTestCase() {
        // 这里可以添加清理代码
        TestHelpers::cleanup();
    }
};

#ifndef RUN_PROCESS_UNIFIED_TESTS
QTEST_MAIN(ProcessMd5SumTest)
#endif

#include "processmd5sum_test.moc"

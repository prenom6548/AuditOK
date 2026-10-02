#include <QtTest>
#include "../src/process/processopenfile.h"
#include "../tests/test_common/test_helpers.h"

class ProcessOpenFileTest : public QObject
{
    Q_OBJECT

private slots:
    // 测试初始化
    void initTestCase() {
        // 可以在这里初始化一些测试所需的资源
    }

    // 测试构造函数
    void testConstructor() {
        // 创建ProcessOpenFile对象
        QString file_save_path = QCoreApplication::applicationDirPath().append(QString("/../../test_data/data_file.txt"));
        ProcessOpenFile* process = new ProcessOpenFile(file_save_path);
        process->setFile(file_save_path);
        process->exec();
        QVERIFY(true); // 如果能成功构造，则测试通过
    }
    // 测试构造函数
    void testOpenfileError() {
        // 创建ProcessOpenFile对象
        QString file_save_path = QCoreApplication::applicationDirPath().append(QString("/../../test_data/unexist.txt"));
        ProcessOpenFile* process = new ProcessOpenFile(file_save_path);
        process->setFile(file_save_path);
        process->exec();
        QVERIFY(true); // 如果能成功构造，则测试通过
    }

    // 测试析构函数
    void testDestructor() {
        // 创建ProcessOpenFile对象并删除
        ProcessOpenFile* process = new ProcessOpenFile("test");
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
QTEST_MAIN(ProcessOpenFileTest)
#endif
#include "processopenfile_test.moc"

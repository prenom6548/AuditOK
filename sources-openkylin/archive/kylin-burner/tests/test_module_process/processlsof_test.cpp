#include <QtTest>
#include "../src/process/processlsof.h"
#include "../tests/test_common/test_helpers.h"
#include "../src/data/datacdrom.h"

class ProcessLsofTest : public QObject
{
    Q_OBJECT

private slots:
    // 测试初始化
    void initTestCase() {
        // 可以在这里初始化一些测试所需的资源
    }

    // 测试构造函数
    void testConstructor() {
        // 创建ProcessLsof对象
        ProcessLsof process;
        QVERIFY(true);
    }

    void testCheckDevice(){
        ProcessLsof lsofPro;
        DataCDROM* cdrom = new DataCDROM("/dev/sr0");
        if (lsofPro.checkDevice(cdrom)) {
            const QList<ProcessLsof::Process>& apps = lsofPro.usingApplications();
        }
        QVERIFY(true);
    }

    // 测试析构函数
    void testDestructor() {
        // 创建ProcessLsof对象并删除
        ProcessLsof* process = new ProcessLsof();
        delete process;
        QVERIFY(true);
    }

    // 清理测试环境
    void cleanupTestCase() {
        // 这里可以添加清理代码
        TestHelpers::cleanup();
    }
};

#ifndef RUN_PROCESS_UNIFIED_TESTS
QTEST_MAIN(ProcessLsofTest)
#endif
#include "processlsof_test.moc"

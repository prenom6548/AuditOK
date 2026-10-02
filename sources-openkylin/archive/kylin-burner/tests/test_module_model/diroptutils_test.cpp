#include <QtTest>
#include "../src/model/diroptutils.h"
#include "../tests/test_common/test_helpers.h"

class DirOptUtilsTest : public QObject
{
    Q_OBJECT

private slots:
    // 测试初始化
    void initTestCase() {
        // 可以在这里初始化一些测试所需的资源
    }



    // 清理测试环境
    void cleanupTestCase() {
        // 这里可以添加清理代码
        TestHelpers::cleanup();
    }
};

QTEST_MAIN(DirOptUtilsTest)
#include "diroptutils_test.moc"

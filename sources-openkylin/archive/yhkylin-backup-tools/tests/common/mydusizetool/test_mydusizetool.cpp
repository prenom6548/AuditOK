#include "test_mydusizetool.h"
#include <QTemporaryDir>
#include <QFile>
#include <QDir>

void TestMyDuSizeTool::initTestCase()
{
    // 初始化测试环境
}

void TestMyDuSizeTool::cleanupTestCase()
{
    // 清理测试环境
}

void TestMyDuSizeTool::testConstructor()
{
    // 测试构造函数
    MyDuSizeTool tool;
    QVERIFY(tool.size() == 0);
}

void TestMyDuSizeTool::testDoWithExcludePaths_data()
{
    QTest::addColumn<QStringList>("paths");
    QTest::addColumn<QStringList>("excludePaths");
    QTest::addColumn<bool>("block");
    QTest::addColumn<qint64>("expectedResult");

    // 测试用例1: 空路径列表
    QTest::newRow("empty paths") << QStringList() << QStringList() << true << -1LL;

    // 测试用例2: 有效路径但空排除列表
    QTest::newRow("empty exclude paths") << QStringList(".") << QStringList() << true << -1LL;

    // 测试用例3: 当前目录
    QTest::newRow("current directory") << QStringList(".") << QStringList("/proc") << true << 0LL;
}

void TestMyDuSizeTool::testDoWithExcludePaths()
{
    QFETCH(QStringList, paths);
    QFETCH(QStringList, excludePaths);
    QFETCH(bool, block);
    QFETCH(qint64, expectedResult);

    MyDuSizeTool tool;
    qint64 result = tool.Do(paths, excludePaths, block);
    
    // 对于非阻塞调用，期望返回0
    if (!block) {
        QCOMPARE(result, 0LL);
    } 
    // 对于阻塞调用且路径无效的情况，期望返回-1
    else if (paths.isEmpty() || excludePaths.isEmpty()) {
        QCOMPARE(result, -1LL);
    }
    // 其他情况验证返回值大于等于0
    else {
        QVERIFY(result >= 0);
    }
}

void TestMyDuSizeTool::testDoWithoutExcludePaths_data()
{
    QTest::addColumn<QStringList>("paths");
    QTest::addColumn<bool>("block");
    QTest::addColumn<qint64>("expectedResult");

    // 测试用例1: 空路径列表
    QTest::newRow("empty paths") << QStringList() << true << -1LL;

    // 测试用例2: 当前目录
    QTest::newRow("current directory") << QStringList(".") << true << 0LL;
    
    // 测试用例3: 非阻塞调用
    QTest::newRow("non-blocking") << QStringList(".") << false << 0LL;
}

void TestMyDuSizeTool::testDoWithoutExcludePaths()
{
    QFETCH(QStringList, paths);
    QFETCH(bool, block);
    QFETCH(qint64, expectedResult);

    MyDuSizeTool tool;
    qint64 result = tool.Do(paths, block);
    
    // 对于非阻塞调用，期望返回0
    if (!block) {
        QCOMPARE(result, 0LL);
    } 
    // 对于阻塞调用且路径无效的情况，期望返回-1
    else if (paths.isEmpty()) {
        QCOMPARE(result, -1LL);
    }
    // 其他情况验证返回值大于等于0
    else {
        QVERIFY(result >= 0);
    }
}

void TestMyDuSizeTool::testSize()
{
    MyDuSizeTool tool;
    // 初始大小应为0
    QCOMPARE(tool.size(), 0LL);
}
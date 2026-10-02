#include "test_utils.h"
#include "../../../common/utils.h"
#include "../../../common/mydefine.h"

void TestUtils::initTestCase()
{
    // 初始化测试环境
}

void TestUtils::cleanupTestCase()
{
    // 清理测试环境
}

void TestUtils::testStringBySize_data()
{
    QTest::addColumn<qint64>("size");
    QTest::addColumn<QString>("expected");

    QTest::newRow("0 bytes") << qint64(0) << QString("0.00KB");
    QTest::newRow("1023 bytes") << qint64(1023) << QString("1.00KB");
    QTest::newRow("1024 bytes") << qint64(1024) << QString("1.00KB");
    QTest::newRow("1024*1024 bytes") << qint64(1024*1024) << QString("1024.00KB");
    QTest::newRow("1024*1024+1 bytes") << qint64(1024*1024+1) << QString("1.00MB");
    QTest::newRow("1024*1024*1024 bytes") << qint64(1024*1024*1024) << QString("1024.00MB");
    QTest::newRow("1024*1024*1024*2 bytes") << qint64(1024LL*1024*1024*2) << QString("2.00GB");
}

void TestUtils::testStringBySize()
{
    QFETCH(qint64, size);
    QFETCH(QString, expected);
    
    QString result = Utils::StringBySize(size);
    QCOMPARE(result, expected);
}

void TestUtils::testSizeByString_data()
{
    QTest::addColumn<QString>("sizeString");
    QTest::addColumn<qint64>("expected");

    QTest::newRow("0B") << QString("0B") << qint64(0);
    QTest::newRow("1KB") << QString("1KB") << qint64(1024);
    QTest::newRow("1.5KB") << QString("1.5KB") << qint64(1536);
    QTest::newRow("1MB") << QString("1MB") << qint64(1024*1024);
    QTest::newRow("2GB") << QString("2GB") << qint64(1024LL*1024*1024*2);
    QTest::newRow("1TB") << QString("1TB") << qint64(1024LL*1024*1024*1024);
}

void TestUtils::testSizeByString()
{
    QFETCH(QString, sizeString);
    QFETCH(qint64, expected);
    
    // qint64 result = Utils::SizeByString(sizeString);
    // QCOMPARE(result, expected);
}

void TestUtils::testIsDirExists()
{
    // 测试存在的目录
    // QVERIFY(Utils::isDirExists("/") == true);
    
    // // 测试不存在的目录
    // QVERIFY(Utils::isDirExists("/this_directory_should_not_exist") == false);
}

void TestUtils::testCreateUuid()
{
    QString uuid1 = Utils::createUuid();
    QString uuid2 = Utils::createUuid();
    
    // 检查UUID不为空
    QVERIFY(!uuid1.isEmpty());
    QVERIFY(!uuid2.isEmpty());
    
    // 两次生成的UUID应该不同
    QVERIFY(uuid1 != uuid2);
    
    // 检查UUID不等于预定义的特殊UUID
    QVERIFY(uuid1 != AUTO_BACKUP_UUID);
    QVERIFY(uuid1 != FACTORY_BACKUP_UUID);
}
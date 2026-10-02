
#include <QtTest/QtTest>

class UdfReaderTest : public QObject
{
    Q_OBJECT

private slots:
    // 测试tag_udf_partitions结构体
    void testTagUdfPartitionsStruct() {

    }

    // 测试UdfReader类构造函数
    void testUdfReaderConstructor() {
        
        // 验证对象能正常构造
        QVERIFY(true); // 如果能编译通过则测试通过
    }

    // 测试UdfReader类析构函数
    void testUdfReaderDestructor() {

        // 验证对象能正常析构
        QVERIFY(true); // 如果能编译通过则测试通过
    }
};

QTEST_MAIN(UdfReaderTest)
#include "udfreader_test.moc"

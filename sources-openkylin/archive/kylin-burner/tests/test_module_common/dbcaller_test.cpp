#include <QtTest>
#include "common/dbuscaller.h"
#include "test_common/test_helpers.h"

class DBusCallerTest : public QObject
{
    Q_OBJECT

private slots:
    // 测试初始化
    void initTestCase() {
        // 可以在这里初始化一些测试所需的资源
    }

    // 测试构造函数
    void testConstructor() {
        DBusCaller caller;
        QVERIFY(true); // 如果能成功构造，则测试通过
    }

    // 测试CallResult枚举值
    void testCallResultEnum() {
        QCOMPARE(DBusCaller::Failure, 1);
        QCOMPARE(DBusCaller::Success, 0);
        QCOMPARE(DBusCaller::Timeout, -1);
        QCOMPARE(DBusCaller::MethodNotExist, -2);
        QCOMPARE(DBusCaller::DBusError, -3);
    }

    // 测试handleSensitiveInfoCheckResult方法
    void testHandleSensitiveInfoCheckResult() {
        DBusCaller caller;
        
        // 测试Success情况
        QVERIFY(caller.handleSensitiveInfoCheckResult(DBusCaller::Success));
        
        // 测试Failure情况
        QVERIFY(!caller.handleSensitiveInfoCheckResult(DBusCaller::Failure));
        
        // 测试Timeout情况
        QVERIFY(!caller.handleSensitiveInfoCheckResult(DBusCaller::Timeout));
        
        // 测试MethodNotExist情况
        QVERIFY(caller.handleSensitiveInfoCheckResult(DBusCaller::MethodNotExist));
        
        // 测试DBusError情况
        QVERIFY(caller.handleSensitiveInfoCheckResult(DBusCaller::DBusError));
        
        // 测试默认情况
        QVERIFY(!caller.handleSensitiveInfoCheckResult(999)); // 任意非预期值
    }

    // 清理测试环境
    void cleanupTestCase() {
        // 这里可以添加清理代码
        TestHelpers::cleanup();
    }
};

QTEST_MAIN(DBusCallerTest)
#include "dbcaller_test.moc"

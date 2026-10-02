#include <QtTest>
#include "../src/frame/logger.h"
#include "../tests/test_common/test_helpers.h"

class LoggerTest : public QObject
{
    Q_OBJECT

private slots:
    // 测试初始化
    void initTestCase() {
        // 可以在这里初始化一些测试所需的资源
    }

    // 测试单例模式
    void testSingleton() {
        Logger* instance1 = Logger::instance();
        Logger* instance2 = Logger::instance();
        QVERIFY(instance1 != nullptr);
        QVERIFY(instance2 != nullptr);
        QCOMPARE(instance1, instance2); // 验证是否为同一实例
    }

    // 测试构造函数和析构函数
    void testConstructorAndDestructor() {
        // 由于Logger是单例模式，我们不能直接构造它
        // 但我们可以通过instance()方法获取实例
        Logger* logger = Logger::instance();
        QVERIFY(logger != nullptr);
    }

    // 测试init方法
    void testInit() {
        Logger* logger = Logger::instance();
        
        // 调用init方法（不会崩溃即可）
        logger->init();
        QVERIFY(true);
    }

    // 测试registerName方法
    void testRegisterName() {
        Logger* logger = Logger::instance();
        
        // 创建一个测试对象
        QObject* testObj = new QObject();
        QString testName = "TestObject";
        
        // 注册对象名称
        QString registeredName = logger->registerName(testObj, testName);
        QVERIFY(!registeredName.isEmpty());
        
        // 清理
        delete testObj;
    }

    // 测试日志方法
    void testLogMethods() {
        Logger* logger = Logger::instance();
        
        QString testMessage = "Test log message";
        
        // 调用各种日志方法（不会崩溃即可）
        logger->info(testMessage);
        logger->warn(testMessage);
        logger->error(testMessage);
        logger->debug(testMessage);
        
        QVERIFY(true);
    }

    // 清理测试环境
    void cleanupTestCase() {
        // 这里可以添加清理代码
        TestHelpers::cleanup();
    }
};

QTEST_MAIN(LoggerTest)
#include "logger_test.moc"
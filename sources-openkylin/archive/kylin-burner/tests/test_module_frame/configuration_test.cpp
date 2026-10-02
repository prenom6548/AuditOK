#include <QtTest>
#include "../src/frame/configuration.h"
#include "../tests/test_common/test_helpers.h"

class ConfigurationTest : public QObject
{
    Q_OBJECT

private slots:
    // 测试初始化
    void initTestCase() {
        // 可以在这里初始化一些测试所需的资源
    }

    // 测试单例模式
    void testSingleton() {
        Configuration* instance1 = Configuration::instance();
        Configuration* instance2 = Configuration::instance();
        QVERIFY(instance1 != nullptr);
        QVERIFY(instance2 != nullptr);
        QCOMPARE(instance1, instance2); // 验证是否为同一实例
    }

    // 测试构造函数和析构函数
    void testConstructorAndDestructor() {
        // 由于Configuration是单例模式，我们不能直接构造它
        // 但我们可以通过instance()方法获取实例
        Configuration* config = Configuration::instance();
        QVERIFY(config != nullptr);
    }

    // 测试init方法
    void testInit() {
        Configuration* config = Configuration::instance();
        
        // 调用init方法（不会崩溃即可）
        config->init();
        QVERIFY(true);
    }

    // 测试get和set方法
    void testGetAndSet() {
        Configuration* config = Configuration::instance();
        
        QString group = "TestGroup";
        QString key = "TestKey";
        QVariant value = "TestValue";
        QVariant defaultValue = "DefaultValue";
        
        // 设置值
        config->set(group, key, value);
        
        // 获取值
        QVariant retrievedValue = config->get(group, key, defaultValue);
        QCOMPARE(retrievedValue, value);
        
        // 测试不存在的键返回默认值
        QVariant retrievedDefaultValue = config->get(group, "NonExistentKey", defaultValue);
        QCOMPARE(retrievedDefaultValue, defaultValue);
    }

    // 测试gets和sets方法
    void testGetsAndSets() {
        Configuration* config = Configuration::instance();
        
        QString group = "TestGroup";
        QString key = "TestListKey";
        QVariantList values;
        values << "Value1" << "Value2" << "Value3";
        
        // 设置值列表
        config->sets(group, key, values);
        
        // 获取值列表
        QVariantList retrievedValues = config->gets(group, key);
        QCOMPARE(retrievedValues, values);
    }

    // 清理测试环境
    void cleanupTestCase() {
        // 这里可以添加清理代码
        TestHelpers::cleanup();
    }
};

QTEST_MAIN(ConfigurationTest)
#include "configuration_test.moc"
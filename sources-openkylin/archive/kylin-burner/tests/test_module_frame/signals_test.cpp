#include <QtTest>
#include "../src/frame/signals.h"
#include "../tests/test_common/test_helpers.h"

class SignalsTest : public QObject
{
    Q_OBJECT

private slots:
    // 测试初始化
    void initTestCase() {
        // 可以在这里初始化一些测试所需的资源
    }

    // 测试GlobalUserSignal单例模式
    void testGlobalUserSignalSingleton() {
        GlobalUserSignal* instance1 = GlobalUserSignal::getInstance();
        GlobalUserSignal* instance2 = GlobalUserSignal::getInstance();
        QVERIFY(instance1 != nullptr);
        QVERIFY(instance2 != nullptr);
        QCOMPARE(instance1, instance2); // 验证是否为同一实例
    }

    // 测试GlobalCoreSignal单例模式
    void testGlobalCoreSignalSingleton() {
        GlobalCoreSignal* instance1 = GlobalCoreSignal::getInstance();
        GlobalCoreSignal* instance2 = GlobalCoreSignal::getInstance();
        QVERIFY(instance1 != nullptr);
        QVERIFY(instance2 != nullptr);
        QCOMPARE(instance1, instance2); // 验证是否为同一实例
    }

    // 测试GlobalConfigSignal单例模式
    void testGlobalConfigSignalSingleton() {
        GlobalConfigSignal* instance1 = GlobalConfigSignal::getInstance();
        GlobalConfigSignal* instance2 = GlobalConfigSignal::getInstance();
        QVERIFY(instance1 != nullptr);
        QVERIFY(instance2 != nullptr);
        QCOMPARE(instance1, instance2); // 验证是否为同一实例
    }

    // 测试GlobalUserSignal构造函数和析构函数
    void testGlobalUserSignalConstructorAndDestructor() {
        GlobalUserSignal* signal = GlobalUserSignal::getInstance();
        QVERIFY(signal != nullptr);
    }

    // 测试GlobalCoreSignal构造函数和析构函数
    void testGlobalCoreSignalConstructorAndDestructor() {
        GlobalCoreSignal* signal = GlobalCoreSignal::getInstance();
        QVERIFY(signal != nullptr);
    }

    // 测试GlobalConfigSignal构造函数和析构函数
    void testGlobalConfigSignalConstructorAndDestructor() {
        GlobalConfigSignal* signal = GlobalConfigSignal::getInstance();
        QVERIFY(signal != nullptr);
    }

    // 测试设备忙状态相关方法
    void testDeviceBusyStatusMethods() {
        GlobalUserSignal* signal = GlobalUserSignal::getInstance();
        
        QString deviceName = "/dev/sr0";
        bool busyStatus = true;
        
        // 设置设备忙状态
        signal->setDeviceBusyStatus(deviceName, busyStatus);
        
        // 获取设备忙状态
        bool retrievedStatus = signal->getDeviceBusyStatus(deviceName);
        QCOMPARE(retrievedStatus, busyStatus);
        
        // 移除设备状态
        signal->removeDevice(deviceName, true);
    }

    // 清理测试环境
    void cleanupTestCase() {
        // 这里可以添加清理代码
        TestHelpers::cleanup();
    }
};

QTEST_MAIN(SignalsTest)
#include "signals_test.moc"
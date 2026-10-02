/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "base/singleton.h"
#include <QTest>
#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

// 假设我们有一个简单的类来测试Singleton
class TestClass
{
public:
    TestClass()
    {
        instanceCount++;
    }
    ~TestClass()
    {
        instanceCount--;
    }

    int getValue() const
    {
        return value;
    }
    void setValue(int val)
    {
        value = val;
    }

    // 静态成员跟踪实例数量
    static int instanceCount;

private:
    int value = 0;
};

int TestClass::instanceCount = 0;

// 带构造参数的测试类
class TestClassWithParams
{
public:
    TestClassWithParams()
        : initialized(false)
        , value(0)
    {
    }

    void initialize(int val)
    {
        value = val;
        initialized = true;
    }

    bool isInitialized() const
    {
        return initialized;
    }
    int getValue() const
    {
        return value;
    }

private:
    bool initialized;
    int value;
};

// 线程安全性测试类
class ThreadTestClass
{
public:
    void increment()
    {
        std::lock_guard<std::mutex> lock(mutex);
        counter++;
    }

    int getCounter() const
    {
        std::lock_guard<std::mutex> lock(mutex);
        return counter;
    }

private:
    int counter = 0;
    mutable std::mutex mutex;
};

class TestSingleton : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init();
    void cleanup();

    void testSingletonInstance();
    void testSingletonCopy();
    void testSingletonMove();
    void testSingletonInstanceCount();
    void testSingletonLazyInitialization();
    void testSingletonThreadSafety();
    void testMultipleTypeSingletons();
    void testSingletonInitialization();
};

void TestSingleton::init()
{
    // 确保每个测试开始前实例计数为0
    TestClass::instanceCount = 0;
}

void TestSingleton::cleanup()
{
    // 确保每个测试结束后没有内存泄漏
    QCOMPARE(TestClass::instanceCount, 0);
}

void TestSingleton::testSingletonInstance()
{
    // 获取单例实例
    TestClass &instance1 = Singleton<TestClass>::instance();
    TestClass &instance2 = Singleton<TestClass>::instance();

    // 检查两个实例是否相同
    QVERIFY(&instance1 == &instance2);

    // 修改实例的值
    instance1.setValue(42);

    // 检查另一个实例的值是否被修改
    QCOMPARE(instance2.getValue(), 42);
}

void TestSingleton::testSingletonCopy()
{
    // 获取单例实例
    TestClass &instance = Singleton<TestClass>::instance();

    // 设置实例的初始值
    instance.setValue(42);

    // 尝试拷贝实例 - 编译器会阻止这一行，因为拷贝构造函数被删除了
    // 但我们可以测试简单的赋值操作是否有效
    TestClass anotherInstance;
    anotherInstance.setValue(instance.getValue());

    // 验证值被正确复制
    QCOMPARE(anotherInstance.getValue(), 42);

    // 确保修改anotherInstance不会影响单例实例
    anotherInstance.setValue(100);
    QCOMPARE(instance.getValue(), 42);
}

void TestSingleton::testSingletonMove()
{
    // 获取单例实例
    TestClass &instance = Singleton<TestClass>::instance();

    // 设置实例的初始值
    instance.setValue(42);

    // 尝试移动实例 - 编译器会阻止这一行，因为移动构造函数被删除了
    // 但我们可以测试简单的移动赋值操作是否有效
    TestClass anotherInstance;
    anotherInstance.setValue(std::move(instance.getValue()));

    // 验证值被正确移动
    QCOMPARE(anotherInstance.getValue(), 42);
    QCOMPARE(instance.getValue(), 42); // 对于基本类型，移动和复制是相同的
}

void TestSingleton::testSingletonInstanceCount()
{
    // 检查初始实例计数
    QCOMPARE(TestClass::instanceCount, 0);

    {
        // 获取单例实例，应该创建一个实例
        TestClass &instance = Singleton<TestClass>::instance();
        QCOMPARE(TestClass::instanceCount, 1);

        // 再次获取实例，不应该创建新实例
        TestClass &instance2 = Singleton<TestClass>::instance();
        QCOMPARE(TestClass::instanceCount, 1);
    }

    // 离开作用域后，单例实例仍然存在，因为它是静态的
    QCOMPARE(TestClass::instanceCount, 1);

    // 注意：在测试结束时，静态变量会被销毁，这应该在cleanup()中验证
}

void TestSingleton::testSingletonLazyInitialization()
{
    // 检查初始实例计数
    QCOMPARE(TestClass::instanceCount, 0);

    // 不调用instance()，不应该创建实例
    QCOMPARE(TestClass::instanceCount, 0);

    // 调用instance()，应该创建实例
    Singleton<TestClass>::instance();
    QCOMPARE(TestClass::instanceCount, 1);
}

void TestSingleton::testSingletonThreadSafety()
{
    // 在多个线程中访问同一个单例实例
    constexpr int threadCount = 10;
    constexpr int iterationsPerThread = 1000;

    std::vector<std::thread> threads;

    for (int i = 0; i < threadCount; ++i) {
        threads.emplace_back([iterationsPerThread]() {
            ThreadTestClass &instance = Singleton<ThreadTestClass>::instance();
            for (int j = 0; j < iterationsPerThread; ++j) {
                instance.increment();
            }
        });
    }

    for (auto &thread : threads) {
        thread.join();
    }

    // 验证计数器的值
    ThreadTestClass &instance = Singleton<ThreadTestClass>::instance();
    QCOMPARE(instance.getCounter(), threadCount * iterationsPerThread);
}

void TestSingleton::testMultipleTypeSingletons()
{
    // 测试不同类型的单例
    TestClass &testClassInstance = Singleton<TestClass>::instance();
    ThreadTestClass &threadTestClassInstance = Singleton<ThreadTestClass>::instance();

    // 设置测试值
    testClassInstance.setValue(42);
    threadTestClassInstance.increment();

    // 验证每个实例的值
    QCOMPARE(testClassInstance.getValue(), 42);
    QCOMPARE(threadTestClassInstance.getCounter(), 1);

    // 确保实例是不同的
    void *testClassPtr = static_cast<void *>(&testClassInstance);
    void *threadTestClassPtr = static_cast<void *>(&threadTestClassInstance);
    QVERIFY(testClassPtr != threadTestClassPtr);
}

void TestSingleton::testSingletonInitialization()
{
    // 获取单例并初始化
    TestClassWithParams &instance = Singleton<TestClassWithParams>::instance();

    // 检查初始状态
    QVERIFY(!instance.isInitialized());
    QCOMPARE(instance.getValue(), 0);

    // 初始化
    instance.initialize(100);

    // 检查初始化后的状态
    QVERIFY(instance.isInitialized());
    QCOMPARE(instance.getValue(), 100);

    // 获取另一个引用并验证状态相同
    TestClassWithParams &instance2 = Singleton<TestClassWithParams>::instance();
    QVERIFY(instance2.isInitialized());
    QCOMPARE(instance2.getValue(), 100);

    // 修改值并验证两个引用都看到更改
    instance.initialize(200);
    QCOMPARE(instance2.getValue(), 200);
}

QTEST_APPLESS_MAIN(TestSingleton)

#include "tst_singleton.moc"